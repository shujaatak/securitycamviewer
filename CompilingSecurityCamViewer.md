# Introduction #

This is a basic guide on compiling SecurityCamViewer using Qt's SDK.

# What to Build #

The SecurityCamViewer source tree contains two distinct executable targets.

  * client - A Qt-based GUI for live feeds from an IP camera and viewing stored recordings
  * muxer - A command-line utility which connects to multiple IP camera feeds and merges them into a single output stream, optionally resizing the feeds to make the final output smaller.

You'll probably want to build the .pro file in the 'client' folder if you're just looking for a quick start.

# Building 'client' #

  1. Download and install Qt from [http://qt.nokia.com/downloads](http://qt.nokia.com/downloads). The LGPL/free version is fine.
  1. If you're using windows, Download and install TortoiseSVN from http://tortoisesvn.net/downloads - any SVN client will do, really. On linux, subversion normally is already installed.
  1. Check out the DViz source code - SVN repository URL is http://securitycamviewer.googlecode.com/svn/trunk/. See the [Source tab](http://code.google.com/p/securitycamviewer/source/checkout) (above) for more details.
  1. Open Qt Creator (Start Menu -> Qt -> Qt Creator )
  1. Use File->Open to open the .pro file in the src/client subfolder from wherever you stored the securitycamviewer source.
  1. From the 'Build' menu in Qt Creator, choose 'Build All'
  1. Now you can run the SecurityCamViewer client by using Qt Creator. Choose the Build menu again and click 'Run.'

# Linux: Running 'client' outside Qt Creator #

  * On linux, just ensure that your LD\_LIBRARY\_PATH includes the install location of Qt. Normally, I do something like this:
    * `export LD_LIBRARY_PATH=/opt/qtsdk-2010.05/qt/lib:$LD_LIBRARY_PATH`
    * In the 'export' command above, be sure you use the correct Qt version and location (qtsdk-2010.05).
  * Also, make sure your QT\_PLUGIN\_PATH includes the version of Qt you build with. I normally do something like this:
    * `export QT_PLUGIN_PATH=/opt/qtsdk-2010.05/qt/plugins:$QT_PLUGIN_PATH`
  * Then, just cd to the 'client' directory where you stored the source and run './client'
  * To configure the viewer, edit the 'viewer.ini' file in the 'client' directory

# Windows: Running 'client' outside Qt Creator #

  * On windows, To run the SecurityCamViewer client outside Qt Creator, you'll need to copy the relevant DLLs from c:\qt\...\qt\bin. The '...' depends on the version of Qt you installed.
  * You'll need mingw10.dll, QtCore4.dll, QtGui4.dll, QtNetwork4.dll, QtOpenGL4.dll, QtScript4.dll, QtSql4.dll, QtSvg4.dll, QtXml4.dll, and QtXmlPatterns4.dll
  * Copy these DLLs to the same folder as client.exe.
  * You'll need the Qt plugins from the Qt install location (C:\qt\...\qt\plugins) inorder to use the song database or use JPEGs on windows.
  * Copy the entire Qt Plugins folder to the same folder as dviz.exe
  * Now you can run the client by just double-clicking on clientexe