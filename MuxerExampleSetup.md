# Introduction #

At [Pleasant Hill Church](http://www.mypleasanthillchurch.org), we have a number of IP cameras setup around the church monitoring various doors and equipments. The cameras are monitored by 'motion', and a muxer instance is used on the same server to merge the camera streams from 'motion' into a single stream and send it via an ssh tunnel to the church's web server for remote viewing. On the server, another muxer instance acts as a simple proxy, allowing multiple clients to connect to the muxer on the webserver, while the muxer on the webserver is the only client of the muxer at the church - thereby keeping the bandwidth used on the uplink to a minimum.

# Details #

Let's take this in stages:

  * Cameras
  * Muxer1
  * Uplink
  * Muxer2
  * Viewers

But first, a cheesy ASCII graphic.

## Cheesy ASCII Graphic Overview ##

The setup looks something like this:
```
                                                 (viewer)
Cam1 --,                                        /
        \                                      /
Cam2 ----[muxer1]---(120kbps uplink)---[muxer2]----(viewer)
        /                                      \
Cam3 --'                                        \
                                                 (viewer)
```


## Cameras ##

All of the cameras are running at 640x480, and streaming around 15-20fps (MJPEG multipart) to the 'motion' server in the equipment room. The motion server itself is setup with each camera thread feeding images out on it's own webcam port. For example, camera 1 is port 8081, cam 2 is 8082, etc.

## Muxer 1 ##

The first muxer instance takes the multiple webcam streams from motion and merges them into a single stream by painting each stream into section of a square. Like:

```
+---------+
|Cam1|Cam2|
+----+----+
|Cam3|Cam4|
+---------+
```

This will work for (just about) any number of cameras - does not have to be a square root-able number. The muxer will automatically arrange the individual streams into a quasi-pleasing arrangement.

The muxer then opens a port and listens for incoming HTTP connections on that port. All you would have to do to view the resulting muxed stream is point your browser to that port, e.g. http://server:8089/. (Firefox and Safari - IE doesn't handle the MJPEG streams by itself.)

**Review**

Alright, where are we right now? We've got all the cameras being monitored by 'motion', and the muxer taking the output webcam streams from 'motion' and merging them to a single stream, and now the muxer is just listening on an open port for clients. Make sense so far? Okay.

## Uplink ##

Just for background's sake, we'll look at the uplink. Due to the location of the church (8 miles out of town), it uses a wireless connection to the internet though a local wireless ISP. Specifically, the church bounces from the church to a local farmer's grain leg, then down to down to a 320' tower, then from that tower a wireless backhaul goes to nearby Winchester, then (still via wireless backhaul) down to Indianapolis, where it exists to fiber on 39th street. Not that any of that is necessary to this discussion, but it's interesting none the less.

Anyway, the slowest part of that chain is the jump from the church to the farmer's grain leg - its measuring at about 120 Kbps right now. Therefore, limiting the amount of uplink traffic is a major goal in this project, while still being able to allow more than one viewer at the webserver end. Hence, the second muxer, next.

## Muxer 2 ##

To connect to the church, we open a ssh tunnel with port forwarding. Something like this:

```
ssh our.church.address -L8089:cameraserver:8089 'ping -i 5 localhost'
```

(Yes, I know ssh could go into the background for me, but I want it foreground because I wrap it in a perl loop to restart the tunnel if it goes down.)

Therefore, now we have a port on the local server (no GatewayPorts enabled though) that connects us back to the muxer running at the church.

The second muxer instance (muxer2 in the graphic above) just connects to muxer1 at the church as if it were a camera stream (since, after all, the muxer feeds out a standards-compliant multipart jpeg stream), and then listens for incoming clients.

By using this second muxer, the burden of duplicating the incoming stream is on the local muxer rather than having to open multiple streams over the slow uplink for each client.

## Viewer ##

The final method of viewing for this stream is just a simple HTML web page. To allow the clients to connect to the muxer on the server without having to open another port on the firewall, Apache's excellent RewriteEngine is used to proxy a URL to the port inside the firewall:

```
RewriteEngine on
RewriteRule ^/some/image/url$      http://muxerserver:8089/ [P,L]
```

The HTML is incredibly trivial:

```
<img src='/some/image/url'>
```

Note that this won't work in IE - IE will need a plugin to view the multpart JPEG stream.

Cheers!