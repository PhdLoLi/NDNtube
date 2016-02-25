NDNtube -- Pre-recorded Video Streaming Project based on Consumer/Producer API
==

Prerequisites
==
These are prerequisites to build NDNtube.

**Required:**
* [boost](http://www.boost.org/)
* [consumer-producer-api](https://github.com/iliamo/Consumer-Producer-API)
* [NFD](https://github.com/named-data/NFD.git)
* [repo-ng](https://github.com/named-data/repo-ng.git)
* [gstreamer-1.0](http://gstreamer.freedesktop.org/data/pkg/osx/1.4.3/)

Prerequisites build instructions
==

Mac OS build considerations
-

Boost
--
sudo port install boost

consumer-producer-api
--
First build and install this API.

NFD
--
First install consumer-producer-api, then install NFD. please from Source, otherwise will cover the installed con-pro-api

repo-ng
--
copy the repo-ng.config into ${SYSCONFDIR}/ndn(default is /usr/local/etc).

gstreamer-1.0
--
downlaod from http://gstreamer.freedesktop.org/data/pkg/osx/1.4.3/

**Installation: please follow the order listed below. Only tested on the 1.4.3, use exactly this version.**
* gstreamer-1.0-1.4.3-universal.pkg  --- the basic gstreamer lib
* gstreamer-1.0-devel-1.4.3-universal.pkg  --- the develpoer gstreamer lib
* gstreamer-1.0-1.4.3-universal-packages.dmg --- some useful plugins, please install all of them except the last one

**Config:**
* Please add /Library/Frameworks/GStreamer.framework/Versions/1.0/lib/pkgconfig to your $PKG_CONFIG_PATH
* e.g. add "export PKG_CONFIG_PATH=/Library/Frameworks/GStreamer.framework/Versions/1.0/lib/pkgconfig:$PKG_CONFIG_PATH" to your .bashrc

Ubuntu 10.04 build
-

Boost
--
```
$ sudo apt-get install libboost-dev
```

Consumer Producer API
--
Follow guide on https://github.com/named-data/Consumer-Producer-API

NFD
--
Follow guide on https://github.com/named-data/NFD

repo-ng
--
Install from https://github.com/named-data/repo-ng
The copy the repoo-ng.config into the NDN configuration directory

GStreamer
--
```bash
$ sudo apt-get install gstreamer1.0-dev libgstreamer-plugins-base1.0-dev\
    gstreamer1.0-plugins-bad-videoparsers gstreamer1.0-plugins-good
```

Build instructions
==
<pre>
$ ./waf configure
$ ./waf
</pre>
