#include "Muxer.h"

#include "JpegServer.h"
#include "MjpegClient.h"

#include <math.h>
#include <QPainter>

#include <QSettings>

#include <QCoreApplication>

#ifdef OPENCV_ENABLED
#include "EyeCounter.h"
#endif

Muxer::Muxer(QString configFile, bool verbose, QObject *parent)
	: QObject(parent)
	, m_jpegServer(0)
	, m_cols(-1)
	, m_rows(-1)
	, m_verbose(verbose)
	#ifdef OPENCV_ENABLED
	, m_counter(0)
	, m_logFilePtr(0)
	#endif
{
	m_jpegServer = new JpegServer();
	m_jpegServer->setProvider(this, SIGNAL(imageReady(QImage*)));
	
	QSettings settings(configFile,QSettings::IniFormat);
	
	if(verbose)
		qDebug() << "Muxer: Reading settings from "<<configFile;
	
	// Load the frame size (the "small" frame - the final image size is computed automatically)
	QString size = settings.value("frame-size","640x480").toString();
	QStringList part = size.split("x");
	m_frameSize = QSize(part[0].toInt(),part[1].toInt());
	
	if(verbose)
		qDebug() << "Muxer: Frame size: "<<m_frameSize.width()<<"x"<<m_frameSize.height();
	
	// Get this here because we set client fps if poll mode
	int fps = settings.value("fps",2).toInt();
	
	// Setup the server-side of the muxer
	int listenPort = settings.value("listen-port",8088).toInt();
	
	if(verbose)
		qDebug() << "Muxer: Attempting to listen on port"<<listenPort;
	
	if (!m_jpegServer->listen(QHostAddress::Any,listenPort)) 
	{
		qDebug() << "JpegServer could not start on port"<<listenPort<<": "<<m_jpegServer->errorString();
		_exit(2);
		return;
	}
	
	#ifdef OPENCV_ENABLED
	bool enableEyeCounting = settings.value("eye-counting","true").toString() == "true";
	
	if(enableEyeCounting)
	{
		m_highlightEyes = settings.value("eye-highlight","true").toString() == "true";
		m_logFile = settings.value("eye-logfile","eyes-log.csv").toString();
	
		m_counter = new EyeCounter();
		
		if(!m_logFile.isEmpty())
		{
			m_logFilePtr = new QFile(m_logFile);
			
			if (!m_logFilePtr->open(QIODevice::WriteOnly | QIODevice::Text))
			{
				qDebug() << "Muxer: Unable to open"<<m_logFile<<" for writing.";
				delete m_logFilePtr ;
				m_logFilePtr  = 0;
			}
		}
	}
	#endif
	
	int numCameras = settings.value("num-cams",0).toInt();
	
	// Load the defaults for use in the cameras section below
	QString mainHost = settings.value("host","localhost").toString();
	int     mainPort = settings.value("port",80).toInt();
	QString mainPath = settings.value("path","/").toString();
	
	if(numCameras == 0 &&
		(! settings.value("host").isNull() ||
		 ! settings.value("port").isNull() ||
		 ! settings.value("path").isNull()))
	{
		numCameras = 1;
		// since the user defined a "main" host/port or path, then
		// add a single camera using those values, since the 
		// load loop below defaults to the main* values read above
	}
	
	if(verbose)
		qDebug() << "Muxer: Using default host"<<mainHost<<", port"<<mainPort<<", path"<<mainPath;
	
	if(verbose)
		qDebug() << "Muxer: Going to read"<<numCameras<<"cameras";
	
	for(int i=0; i<numCameras;i++)
	{
		// Setup all the threads 
		QString group = QString("cam%1").arg(i);
		
		QString hostKey = QString("%1/host").arg(group);
		QString portKey = QString("%1/port").arg(group);
		QString pathKey = QString("%1/path").arg(group);
		QString flipKey = QString("%1/flip").arg(group);
		QString pollKey = QString("%1/poll").arg(group);
		QString fpsKey  = QString("%1/fps").arg(group);
		
		QString host = settings.value(hostKey,mainHost).toString();
		int     port = settings.value(portKey,mainPort).toInt();
		QString path = settings.value(pathKey,mainPath).toString();
		bool    flip = settings.value(flipKey,0).toInt() == 1;
		bool    poll = settings.value(pollKey,0).toInt() == 1;
		int     cfps = settings.value(fpsKey,fps).toInt();
		
		MjpegClient * client = new MjpegClient();
		client->connectTo(host,port,path);
			
		client->setAutoReconnect(true);
		client->setAutoResize(m_frameSize);
		client->setFlipImage(flip);
		client->setPollingMode(poll);
		client->setPollingFps(cfps);
		client->start();
		
		m_threads    << client;
		m_images     << QImage();
		m_wasChanged << false;
		
		m_counts     << 0;
		m_durations  << 0;
		
		QTime t;
		t.start();
		m_time       << t;
		
		connect(client, SIGNAL(newImage(QImage)), this, SLOT(newImage(QImage)));
		
		if(verbose)
			qDebug() << "Muxer: Setup camera "<<i<<" using host"<<host<<", port"<<port<<", path"<<path;
	}
	
	// Attempt to find an optimum window size to view the cameras in a nice symetric arragmenet
	int numItems = m_images.size();
	
	if(!numItems)
	{
		qDebug() << "No cameras listed in muxer.ini, exiting.";
		_exit(3);
	}
	
	double sq = sqrt(numItems);
	if(((int)sq) != sq)
	{
		// first, attempt to round up
		int x = (int)ceil(sq);
		int y = (int)floor(sq);
		
		if(x*y >= numItems)
			// good to go, apply size
			applySize(x,y);
		else
		{
			// add one row then try
			y++;
			if(x*y >= numItems)
				applySize(x,y);
			else
				// just use the sqrt ceil
				applySize(x,x);
		}
	}
	else
	{
		applySize((int)sq,(int)sq);
	}
	
	if(verbose)
	{
		QSize size = m_muxedImage.size();
		qDebug() << "Muxer: Final image size is"<<size.width()<<"x"<<size.height();
	}
	
	// Setup the frame generation timer 
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateFrames()));
	
	m_updateTimer.setInterval(1000/fps);
	m_updateTimer.start();
	
	if(verbose)
		qDebug() << "Muxer: Running at"<<fps<<" frames per second";
}

Muxer::~Muxer()
{
	while(!m_threads.isEmpty())
	{
		MjpegClient * client = m_threads.takeFirst();
		client->quit();
		client->wait();
		delete client;
		client = 0;
	}
	
	delete m_jpegServer;
}

void Muxer::applySize(int x, int y)
{
	m_cols = x;
	m_rows = y;
	int xpx = x * m_frameSize.width();
	int ypx = y * m_frameSize.height();

	m_muxedImage = QImage(xpx,ypx,QImage::Format_RGB32);
	
	QPainter painter(&m_muxedImage);
	painter.fillRect(m_muxedImage.rect(), Qt::gray);
	painter.end();
	
	emit imageReady(&m_muxedImage);
}


void Muxer::newImage(QImage image)
{
	MjpegClient * client = dynamic_cast<MjpegClient*>(sender());
	if(client)
	{
		int index = m_threads.indexOf(client);
		m_images[index] = image;
		m_wasChanged[index] = true;
//		qDebug() << "newImage(): Received image for thread index"<<index;
		
		m_counts[index] ++;
		
		int time =  m_time[index].restart();
		m_durations[index] += time;
		
		
		if(m_verbose)
		{
			qDebug() << "Muxer: Received image from camera # "<<index << " at "<<(((double)time)/1000.0)<<"sec, avg duration:"<<(m_durations[index] / m_counts[index]);
		}
			
		
		#ifdef OPENCV_ENABLED
		if(m_counter)
		{
			QList<EyeCounterResult> faces = m_counter->detectEyes(image, true);
			
			QPainter painter(&image);
			
			int facesWithEyesCount;
			int eyesCount;
			foreach(EyeCounterResult res, faces)
			{
				if(!res.allEyes.isEmpty())
					facesWithEyesCount ++;
				eyesCount += res.allEyes.size();
				 
				if(m_highlightEyes)
				{
					painter.setPen(Qt::red);
					painter.drawRect(res.face);
					
					painter.setPen(Qt::green);
					foreach(QRect eye, res.allEyes)
						painter.drawRect(eye);
				}
			}
			
			if(m_logFilePtr)
			{
				QTextStream out(m_logFilePtr);
				out << QDateTime::currentDateTime ().toString("yyyy-dd-MM hh:mm:ss") << "," << faces.size() << facesWithEyesCount << eyesCount;
			}
			
		}
		#endif	
	}
}

void Muxer::updateFrames()
{
	QPainter painter(&m_muxedImage);
	
	bool changed = false;
	
	int length = m_images.size();
	for(int i=0;i<length;i++)
	{
		bool flag = m_wasChanged.at(i);
		if(flag)
		{
			QImage image = m_images.at(i);
			double frac = (double)i / (double)m_cols;
			int row = (int)frac;
			int col = (int)((frac - row) * m_cols);
			
			int x = col * m_frameSize.width();
			int y = row * m_frameSize.height();
// 			qDebug() << "updateFrames(): Painted image index "<<i<<" at row"<<row<<", col"<<col<<", coords "<<x<<"x"<<y;
			painter.drawImage(x,y, image);
			
			m_wasChanged[i] = false;
			
			changed = true;
		}
	}
	
	painter.end();
	
	if(changed)
	{
		if(m_verbose)
			qDebug() << "Muxer: Transmitted new frame to clients";
		emit imageReady(&m_muxedImage);
	}
}

