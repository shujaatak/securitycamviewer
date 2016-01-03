Simple security camera viewer based on the Qt library from Trolltech. Designed specifically to be used be with 'motion' project at http://www.lavrsen.dk/twiki/bin/view/Motion/WebHome, though it may be compatible with other security systems.

## Important Note on Viewing Recorded Footage ##

SecurityCamViewer currently is designed to play back jpeg images in quick succession - it only plays jpeg files, NOT video files. It could be a simple code update to adapt the program to play videos instead of jpegs. However, for now, jpegs are the only type of recordings that this program plays. If you would like to sponsor development of video playback, feel free to contact me at josiahbryan@gmail.com.

## SVN [r40](https://code.google.com/p/securitycamviewer/source/detail?r=40) - Windows 7 Build ##
**2012-06-27**

Even though I paused "active" development as of [r31](https://code.google.com/p/securitycamviewer/source/detail?r=31), I still use this program every day and fix issues and make changes as they come up. Since [r31](https://code.google.com/p/securitycamviewer/source/detail?r=31), I've added a few minor improvements: 'flip' support (for upside-down cameras - e.g. ceiling-mounted, etc.), HTTP authentication support, and a few other minor fixes.

Also, a Windows 7 build of the client "viewer" is available for download, via the Downloads tab above, featured downloads on the left, and at this link: http://code.google.com/p/securitycamviewer/downloads/detail?name=securitycamviewer-r40.zip

Here's a screenshot of the main viewer window:

![![](http://securitycamviewer.googlecode.com/svn/trunk/src/screenshots/SecurityCamViewer-small.jpg)](http://securitycamviewer.googlecode.com/svn/trunk/src/screenshots/SecurityCamViewer.jpg)

And here's the "Recorded Footage" window:

![![](http://securitycamviewer.googlecode.com/svn/trunk/src/screenshots/SecurityCamViewer-rec-small.jpg)](http://securitycamviewer.googlecode.com/svn/trunk/src/screenshots/SecurityCamViewer-rec.jpg)

If you have any questions or if you would like to request features, please don't hesitate to contact me at [josiahbryan@gmail.com](mailto:josiahbryan@gmail.com). Cheers!

## SVN [r31](https://code.google.com/p/securitycamviewer/source/detail?r=31) - Development Complete ##
**2010-02-11**

As of [r31](https://code.google.com/p/securitycamviewer/source/detail?r=31), I'm pausing active feature development to invest more time on another project ([dviz](http://code.google.com/p/dviz/)). I consider this project a success and I believe it has accomplished the goals which I set out for it. The primary purpose in writing this code was to provide (a) an easy way to remotely view multiple security cameras over a slow uplink (120kbps) by multiple people without overly taxing the uplink and (b) provide an easy "recorded footage" viewer for reviewing recordings.

Currently the code in this project is actively in use at the church I attend to provide remote viewing for the IP cameras via two muxer instances and two ssh tunnels. See [MuxerExampleSetup](http://code.google.com/p/securitycamviewer/wiki/MuxerExampleSetup) for more details on the remote viewing.

On site, they are monitored and recorded by motion (lavrsen.dk), and recordings are stored on site. For viewing the recordings, the /src/client viewer is used. See the /src/client/viewer.ini for example recording configuration.

If you have any questions or if you would like to request features, please contact me at [josiahbryan@gmail.com](mailto:josiahbryan@gmail.com). Cheers!

## SVN [r29](https://code.google.com/p/securitycamviewer/source/detail?r=29) - Feature Complete ##
**2010-02-09**

As of [r29](https://code.google.com/p/securitycamviewer/source/detail?r=29), the muxer and client viewer are both working as advertised.

Muxer: Adaptive output turned on by default to prevent overwhelming slow uplinks and causing the viewer to fall behind. This can happen when the transmission time for a single frame falls below the frame rate for that frame. E.g. if fps is 2, then if transmission for a single frame falls below 500ms, the viewer will eventually fall behind and grow further and further behind in the video stream. Adaptive output sends a frame down the pipe then waits for the client to receive it by monitoring the socket's output buffers, skipping any new frames that arrive from the cameras until the current frame is transmitted. This will result in dropped frames, but the viewer will be able to receive and stay in time with the server. Still trying to hunt down a problem in the muxer that causes it to crash once in a while, but running it in a loop (e.g. `perl -e 'while(1){system("securitycam-muxer -s -c mymuxer.ini");sleep 1}'`) handles the problem for now.

Client: Playback works by assuming that images are at least segregated by day. Now, that can be any number of actual folder layouts:

  * cam%t/%Y%m%d/%H/%M/%S
  * cam%t/%Y%m%d/%H%M%S
  * cam%t/%Y/%m/%d/%H-%M-%S

You get the idea. The playback location can be specified per camera in the INI file, or it can be specified by giving a folder pattern and letting the viewer 'guess' the motion thread # by applying simple math to the port number of the incoming stream. See the viewer.ini in [r29](https://code.google.com/p/securitycamviewer/source/detail?r=29) for examples of playback specification. Note that though the 'motion' group is included in the INI file, it means nothing right now.

No formal release yet, but [r29](https://code.google.com/p/securitycamviewer/source/detail?r=29) is stable and compiles against Qt 4.6.1 with no external dependencies that I know of (other than Qt itself.) Compiled and tested on Fedora 11, CentOS 5.3, and Windows XP Pro. (To be fair, I havn't tested on XP Pro in about 10 revs - but should work still.)


## Design Description ##

This project is designed to have two primary components:

- Client Viewer
> - A cross-platform client program for (a) viewing camera feeds and (b) playing back stored footage (assuming access to the storage location via a shared network folder, etc)
- Web Viewer
> - Camera stream muxer - a linux Qt executable for headless execution, grab all the streams to be viewed and mix them into a single stream, compressing and resizing as needed, then providing that as a MJPEG stream for viewing over the network.

> - A web front-end written for viewing remote streams.

Note that this project is NOT designed to handle the recording of the images, motion detection, alarm triggers. It is purely a viewer/playback system.

This has the side effect of requiring you to add/change settings by editing the back end recording system - in my case, changing the motion.conf and associated thread files. This viewer/playback project just views and plays back the streams from the 'motion' program (or other possibly compatible back-end recording system.)