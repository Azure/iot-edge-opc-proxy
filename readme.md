This project has adopted the Microsoft Open Source Code of Conduct. For more information see the Code of Conduct FAQ or contact opencode@microsoft.com with any additional questions or comments.

# Beta Azure IoT Gateway Proxy Module

This module is a stream proxy for Azure IoT written in C and intended to be hosted in a Edge Gateway built using the [Azure IoT Gateway SDK](https://github.com/Azure/azure-iot-gateway-sdk).  

It can tunnel streams (at this point TCP based streams only), from a cloud service via one or more local network gateways to a server, using IoT Hub Methods as protocol layer and 
IoT Hub device registry as a name service.  This allows cloud services to send and receive raw buffers to and from local IoT server devices for command and control scenarios
without implementing new protocol translation at the edge.    

Visit http://azure.com/iotdev to learn more about developing applications for Azure IoT.


## Azure IoT Gateway SDK compatibility

Current version of the module is targeted at the Azure IoT Gateway SDK 2016-12-16 version.
Use the following script to download the compatible version Azure IoT Gateway SDK.
```
git clone -b "2016-12-16" --recursive https://github.com/Azure/azure-iot-gateway-sdk.git
```

## Setup and build

This module depends on several other components which are included as submodules. 

### Windows

The module was successfully built and tested on Windows 10 with Visual Studio 2015.

Install CMAKE from https://cmake.org/.

If you are targeting Windows 7, you need to use libwebsockets, and thus also install OpenSSL so that CMAKE can find it. 

### Linux

The module was successfully built and tested on Ubuntu 16.04.  

Install CMAKE, and other dev tools using apt-get, as well as OpenSSL 1.0.1+ and pthreads:
```
sudo apt-get install git cmake libcurl4-openssl-dev build-essential
```

### Building

After cloning the repository, and installing all required tools and dependencies, run build.cmd on Windows, or build.sh on Linux.


## Future direction

We plan on enhancing the following areas over the coming months:

- Re-Enable C API and C sample (currently disabled)
- UDP and forward proxy support (currently untested and likely not working)
- Increased test coverage, in particular end to end tests
- Better PAL abstractions for socket and web
- VNC and BacNET samples
- Documentation
- Appveyor and Travis CI

Contributions are welcome, e.g. in the following areas 

- Better file system abstractions
- RTOS, OSX and Unix ports
- Serial port remoting
- TUN IF tunnel integration
- node.js API

