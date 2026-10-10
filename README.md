# drm-vncserver

VNC server for Linux DRM framebuffer devices.

This project allows remote access to Linux systems with a direct frame buffer rendering.

Working configurations: 32 bits/pixel with 90 degress rotation.
Other resolutions are not implemented.

The code is based on a LibVNC example for Android:
https://github.com/LibVNC/libvncserver/blob/ef971860233a9abfe66533381e82ed094815a83f/examples/androidvncserver.c
and was imported from the following project https://github.com/ponty/framebuffer-vncserver/tree/ad5a7988ade60c3824b68e2908491df4c4164c57/src

### build and install

Dependency:

apt-get install libvncserver-dev libdrm-dev

Run `make` to compile the binary from the `src/` directory.

Run `make install` to install the binary to system.

### command-line help

	drm-vncserver [-f device] [-p port] [-t touchscreen] [-m mouse] [-k keyboard] [-r rotation] [-R touchscreen rotation] [-F FPS] [-v] [-h]
	-p port                VNC port, default is 5900
	-f device              drm device node, default is /dev/dri/card1
	-k device              keyboard device node (example: /dev/input/event0)
	-t device              touchscreen device node (example:/dev/input/event2)
	-m device              mouse device node (example:/dev/input/event2)
	-r degrees             framebuffer rotation, default is 0.
	-R degrees             touchscreen rotation, default is same as framebuffer rotation
	-F                     FPS Maximum target FPS. Default is 0, meaning unlimited FPS.
	-v                     verbose
	-h                     print this help
	-rfbport port          TCP port for RFB protocol
	-rfbportv6 port        TCP6 port for RFB protocol
	-rfbwait time          max time in ms to wait for RFB client
	-rfbauth passwd-file   use authentication on RFB protocol
	                       (use 'storepasswd' to create a password file)
	-rfbversion 3.x        Set the version of the RFB we choose to advertise
	-permitfiletransfer    permit file transfer support
	-passwd plain-password use authentication 
	                       (use plain-password as password, USE AT YOUR RISK)
	-deferupdate time      time in ms to defer updates (default 40)
	-deferptrupdate time   time in ms to defer pointer updates (default none)
	-desktop name          VNC desktop name (default "LibVNCServer")
	-alwaysshared          always treat new clients as shared
	-nevershared           never treat new clients as shared
	-dontdisconnect        don't disconnect existing clients when a new non-shared
	                       connection comes in (refuse new connection instead)
	-sslkeyfile path       set path to private key file for encrypted WebSockets connections
	-sslcertfile path      set path to certificate file for encrypted WebSockets connections
	-httpdir dir-path      enable http server using dir-path home
	-httpport portnum      use portnum for http connection
	-httpportv6 portnum    use portnum for IPv6 http connection
	-enablehttpproxy       enable http proxy support
	-progressive height    enable progressive updating for slow links
	-listen ipaddr         listen for connections only on network interface with
	                       addr ipaddr. '-listen localhost' and hostname work too.
	-listenv6 ipv6addr     listen for IPv6 connections only on network interface with
	                       addr ipv6addr. '-listen localhost' and hostname work too.

### example with a i915 based laptop

	drm-vncserver -f /dev/dri/card1 -t /dev/input/event5 -k /dev/input/event3 -r 90 -F 10
	
	__ __| |           |  /_) |     ___|             |           |
	  |   __ \   _ \  ' /  | |  / |      _ \ __ \   |      _` | __ \   __|
	  |   | | |  __/  . \  |   <  |   |  __/ |   |  |     (   | |   |\__ \
	 _|  _| |_|\___| _|\_\_|_|\_\\____|\___|_|  _| _____|\__,_|_.__/ ____/
	----------------------------------------------------------------------
	TKGL VNC SERVER FOR DRM DEVICES - V1.0
	(c) The KikGen Labs.
	
	[tkgl INFO    ]  VNCSERVER STARTING...
	[tkgl INFO    ]  DRM device /dev/dri/card1 sucessfully opened.
	[tkgl INFO    ]  DRM device has 6 connectors.
	[tkgl INFO    ]  DRM Device Name: eDP-1
	[tkgl INFO    ]  Encoder        : 506
	[tkgl INFO    ]  Resolution : 1920x1200@60
	[tkgl INFO    ]  (ht: 2104 hs: 1936 he: 1952 hskew: 0, vt: 1236  vs: 1203 ve: 1217 vscan: 0, flags: 0xA <P>)
	[tkgl INFO    ]  Connector 507 is connected to encoder 506 CRTC 504.
	[tkgl INFO    ]  Got framebuffer at CRTC: 504.
	[tkgl INFO    ]  FB is format XR24 width 1920 height 1200.
	[tkgl INFO    ]  FB has 3 plane(s):
	[tkgl INFO    ]  Plane 0 has handle 1 pitch 7680 offset 0
	[tkgl INFO    ]  Plane 1 has handle 1 pitch 960 offset 9338880
	[tkgl INFO    ]  Plane 2 has handle 1 pitch 64 offset 9375744
	[tkgl INFO    ]  FB has modifier Intel Color Control Surface with Clear Color (CCS) for DG2 render compression.
	[tkgl INFO    ]   fb xres=1920, yres=1200, bpp=32
	[tkgl INFO    ]    offset:length red=16:255 green=8:255 blue=0:255 
	[tkgl INFO    ]    frame buffer size : 9216000 bytes
	[tkgl INFO    ]  DRM frame buffer map of 9216000 bytes allocated at 0x7070aacaa000.
	[tkgl INFO    ]  Initializing keyboard device /dev/input/event3 ...
	[tkgl INFO    ]  Initializing touch device /dev/input/event5 ...
	[tkgl INFO    ]    x:(0 3364)  y:(0 1669) 
	[tkgl INFO    ]  VNC (TKGL) server initialized with the following parameters :
	[tkgl INFO    ]    width,height       : 1920,1200
	[tkgl INFO    ]    port               : 5900
	[tkgl INFO    ]    rotate             : 90
	[tkgl INFO    ]    mouse/touch rotate : 90
	[tkgl INFO    ]    target FPS         : 10
	[tkgl INFO    ]  Initializing VNC server...
	11/10/2026 15:00:57 Listening for VNC connections on TCP port 5900
	11/10/2026 15:00:57 Listening for VNC connections on TCP6 port 5900	
