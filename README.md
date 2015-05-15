NDNtube -- Pre-recorded Video Streaming
----

NDN Pre-recorded Video Streaming Project based on Consumer/Producer API

For license information see LICENSE.

http://named-data.net/

Description
----

NDN-based Pre-recorded Streaming Video Project using Consumer/Producer API. 


- Producer
    - The repo\_producer will publish all the video files frame by frame and also stream info into Repo-ng.
    - The list\_producer will publish the latest playing list every minute. 

- Consumer
    - The consumer should first ask for the latest playing list, then chose one to play back. 

Structure
----

The directory structure is as follows:

* **/root**
    * **src/** *-- source code*
    * **waf** *-- binary waf file*
    * **wscript** *-- waf instruction file*
    * **repo-ng.config** *-- repo-ng config file, the default path is "/usr/local/etc/ndn/"*
    * **config.ini** *-- config file of this project*
    * **.waf-tools/** *-- additional waf tools*
    * **examples/** *-- no use now*
    * **tests/** *-- no use now*
    * **LICENSE**
    * **README.md**
    * **INSTALL.md** 


Building instructions
----
Please, see the [INSTALL.md](INSTALL.md) for build instructions.

Config
--
- You should first set the "path" and "prefix" value in the config.ini
    - The "path" stands for the path of video files(producer part), we only support MP4 format. Please add some mp4 files into this path, when you run repo\_producer, these video files will be published into repo. You can add new file into this path, just keep repo\_producer running, it will publish the new-added file automatically.
    - The "prefix" stands for the routable ndn prefix

Run
--
Please keep NFD and Repo-ng Running.

- Terminal 1 for repo\_producer -- Firstly, Publishing video & audio to Repo-ng
<pre>
$ .build/repo_producer
</pre>

- Terminal 2 for list\_producer -- Secondely, Publishing playing list
<pre>
$ .build/producer
</pre>

- Terminal 3 -- Playing video & audio
<pre>
$ .build/consumer
</pre>

License
---
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version, with the additional exemption that compiling, linking, and/or using OpenSSL is allowed.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
