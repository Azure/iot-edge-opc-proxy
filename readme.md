This project has adopted the Microsoft Open Source Code of Conduct. For more information see the Code of Conduct FAQ or contact opencode@microsoft.com with any additional questions or comments.

# Azure IoT Field Gateway OPC-UA-Proxy Module

This module is a OPC-UA command and control proxy for Azure IoT written in C and intended to be hosted in an Edge Gateway built using the
[Azure IoT Gateway SDK](https://github.com/Azure/azure-iot-gateway-sdk).  

It can tunnel binary channel streams (at this point TCP based streams only) at the application level, from a cloud service via one or more local
network gateways to a server in the local network.  It uses IoT Hub Methods as protocol layer and IoT Hub device registry as a name 
service provider, and only requires port 443 (outgoing) open to Azure.

This allows cloud OPC-UA applications to send and receive raw buffers to and from IoT server devices that are accessible on the local
gateway network.  Using the API one can quickly implement command and control scenarios from a cloud application 
without providing edge command/control translation.   

Visit http://azure.com/iotdev to learn more about developing applications for Azure IoT.

# Setup and build

|Branch|Status|
|------|-------------|
|master|[![Build status](https://ci.appveyor.com/api/projects/status/do87bhdyyykf6sbj/branch/master?svg=true)](https://ci.appveyor.com/project/marcschier/iot-gateway-opc-ua-proxy/branch/master) [![Build Status](https://travis-ci.org/Azure/iot-gateway-opc-ua-proxy.svg?branch=master)](https://travis-ci.org/Azure/iot-gateway-opc-ua-proxy)|

The Azure IoT Field Gateway Proxy module depends on several components which are included as submodules. Hence, if you did
not specify the ```--recursive``` option when using ```git clone``` to clone this repo, you need to first run ```git submodule update --init```
in a console or terminal window before continuing...

## Linux
- The Proxy module was tested on Ubuntu 16.04 and Alpine Linux 3.5, but can be built on a variety of Linux flavors. Check out the [Dockerfile folder](/bld/docker) to find examples 
on how to set up your specific distribution.  If you do not find yours in the folder, consider contributing a Dockerfile target to this project.
- To build the dotnet samples and API, follow the instructions [here](https://www.microsoft.com/net/core#linuxubuntu) to install .net Core on Linux.
- Run ```bash <repo-root>/bld/build.sh```.  After a successful build, all proxy binaries can be found under the /build/cmake/bin folder.
- (Optional) To install run the usual ```make install``` in directory build/cmake/Release or build/cmake/Debug. For a test install to /tmp/azure you could use for example the following: ```make -C <repo-root>/build/cmake/Debug  DESTDIR=/tmp/azure install```. Then run the proxy: ```LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/tmp/azure/usr/local/lib /tmp/azure/usr/local/bin/proxyd --help```

Run the build script with the ```--help``` option to see all configuration options available.

## Windows
The Proxy module was successfully built and tested on Windows 10 with Visual Studio 2015.
- Install [Visual Studio 2015](https://www.visualstudio.com/downloads/).
- Install CMAKE from [here](https://cmake.org/).  
- Run ```<repo-root>\bld\build.cmd```.  After a successful build, all proxy binaries can be found under the 
\build\cmake\<platform>\bin folder.

Run the build script with the ```--help``` option to see all configuration options available.

# Azure IoT Gateway SDK compatibility
The current version of the Proxy module is targeted at the Azure IoT Gateway SDK 2016-12-16 version.

Use the following command line to clone the compatible version Azure IoT Gateway SDK, then follow the build instructions included:
```
git clone -b "2016-12-16" --recursive https://github.com/Azure/azure-iot-gateway-sdk.git
```

# Samples

## Proxy Module Host Sample

While the Proxy module can be hosted by the field gateway, for demonstration purposes the ```proxyd``` executable can also be used.  The sample requires an
Azure IoT Hub to be provisioned in your Azure Subscription.  For more information, check out the following resources: 

- [Set up IoT Hub](https://github.com/Azure/azure-iot-sdks/blob/master/doc/setup_iothub.md) describes how to configure your Azure IoT Hub service.
- [Manage IoT Hub](https://github.com/Azure/azure-iot-sdks/blob/master/doc/manage_iot_hub.md) describes how to provision devices in your Azure IoT Hub service.

If you run the proxy host sample without a command line argument, the sample tries to read the ```iothubowner``` policy connection strings from the  ```_HUB_CS ```
environment variable, and if successful, will automatically create a proxy entry in your Iot Hub under the host name of the machine it is
running on, and then wait and listen.  It will immediately exit if the variable is not set, or the information is not provided otherwise.
Alternatively, it can read the connection string from a file.  The file name then needs to be provided through the ```-C``` command line argument.  
Using the ```-n``` option it is possible to provide a different proxy identity than the host name of the machine (e.g. to run multiple proxies simultaneously).  
Run the ```proxyd``` executable with ```--help``` command line switch to see all available options. 

A docker container that contains the proxy module host can be built and run directly from github.  If you have not done so already,
install docker on your machine, then in a terminal or console window, run:

```
docker build -t <tag> https://github.com/Azure/iot-gateway-proxy
docker run -e _HUB_CS=<iothubowner-connection-string> <tag>
```

## API Samples

The following samples show how the Gateway module can be used:

- An [OPC UA client](/api/csharp/samples/opc-ua/readme.md) that shows how the [OPC-Foundation reference stack](https://github.com/OPCFoundation/UA-.NETStandardLibrary) transport adapter can provide
OPC-UA relay access to OPC UA servers in a Proxy network.  

- A [Simple TCP/IP services client](/api/csharp/samples/simple/readme.md) that demonstrates the different API calls.

# License

The Azure IoT Field Gateway Proxy module is licensed under the [MIT License](https://github.com/Azure/iot-gateway-proxy/blob/master/LICENSE).  
You can find license information for all third party dependencies [here](https://github.com/Azure/iot-gateway-proxy/blob/master/thirdpartynotice.txt). 
Note that not all of these dependencies need to be utilized, depending on your build configuration, or your choice of platform.

# Support

If you are having issues compiling or using the code in this project please feel free to log an issue in the [issues section](https://github.com/Azure/iot-gateway-proxy/issues) of this project.
For other issues, such as Connectivity issues or problems with the portal, or issues using the Azure IoT Hub service the Microsoft Customer Support team will try and help out on a best effort basis.
To engage Microsoft support, you can create a support ticket directly from the [Azure portal](https://ms.portal.azure.com/#blade/Microsoft_Azure_Support/HelpAndSupportBlade).

# Contributing

Contributions are welcome, in particular in the following areas 

- RTOS, OSX and Unix PAL ports and testing
- None IP streams, e.g. Pipe and Serial port remoting
- API bindings, e.g. node.js or Java

