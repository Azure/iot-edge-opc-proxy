This project has adopted the Microsoft Open Source Code of Conduct. For more information see the Code of Conduct FAQ or contact opencode@microsoft.com with any additional questions or comments.

# Azure IoT Edge OPC Proxy Module

This module is a OPC-UA command and control proxy for Azure IoT written in C and intended to be hosted in an Edge Gateway built using Azure IoT Edge.

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
|master|[![Build status](https://ci.appveyor.com/api/projects/status/do87bhdyyykf6sbj/branch/master?svg=true)](https://ci.appveyor.com/project/marcschier/iot-edge-opc-proxy/branch/master) [![Build Status](https://travis-ci.org/Azure/iot-edge-opc-proxy.svg?branch=master)](https://travis-ci.org/Azure/iot-edge-opc-proxy)|

The Azure IoT Edge OPC Proxy module depends on several components which are included as submodules. Hence, if you did
not specify the ```--recursive``` option when using ```git clone``` to clone this repo, you need to first run 
```git submodule update --init``` in a console or terminal window before continuing...

## Linux
- The Proxy module was tested on Ubuntu 16.04 and Alpine Linux 3.5, but can be built on a variety of Linux flavors. Check out 
the [Dockerfile folder](/bld/docker) to find examples 
on how to set up your specific distribution.  If you do not find yours in the folder, consider contributing a Dockerfile 
target to this project.
- To build the dotnet samples and API, follow the instructions [here](https://www.microsoft.com/net/core#linuxubuntu) to 
install .net Core on Linux.
- Run ```bash <repo-root>/bld/build.sh```.  After a successful build, all proxy binaries can be found under the 
/build/cmake/bin folder.
- (Optional) To install run the usual ```make install``` in directory build/cmake/Release or build/cmake/Debug. For a test 
install to /tmp/azure you could use for example the following: ```make -C <repo-root>/build/cmake/Debug  DESTDIR=/tmp/azure 
install```. Then run the proxy: ```LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/tmp/azure/usr/local/lib /tmp/azure/usr/local/bin/proxyd 
--help```

Run the build script with the ```--help``` option to see all configuration options available.

## Windows
The Proxy module was successfully built and tested on Windows 10 with Visual Studio 2017.
- Install [Visual Studio 2017](https://www.visualstudio.com/downloads/).
- Install CMAKE (3.7 or later) from [here](https://cmake.org/).  
- Run ```<repo-root>\bld\build.cmd```.  After a successful build, all proxy binaries can be found under the 
\build\cmake\<platform>\bin folder.

Run the build script with the ```--help``` option to see all configuration options available.

# Azure IoT Edge compatibility
The current version of the Proxy module is targeted at the Azure IoT Edge 2016-12-16 version.

Use the following command line to clone the compatible version Azure IoT Edge, then follow the build instructions included:
```
git clone -b "2016-12-16" --recursive https://github.com/Azure/iot-edge.git
```
# Samples

All samples require an Azure IoT Hub to be provisioned in your Azure Subscription and access to the ```iothubowner``` 
policy connection string.

For more information, see

- [Set up IoT Hub](https://github.com/Azure/azure-iot-device-ecosystem/blob/master/setup_iothub.md) describes how to 
configure your Azure IoT Hub service.
- [Manage IoT Hub](https://github.com/Azure/azure-iot-device-ecosystem/blob/master/manage_iot_hub.md) describes how to 
provision devices in your Azure IoT Hub service.

For simplicity, the default Iot Hub provider used by most of the samples reads the connection string from the  ```_HUB_CS ``` environment variables when not provided programmatically.  

## Proxy Host 

While the Proxy module can be hosted by a Gateway built using Azure IoT Edge, the ```proxyd``` executable is an 
alternative host which can be configured from the command line.  An IoT Hub connection string can be provided via the 
```-c```, ```-C``` or ```-s``` command line arguments or through the ```_HUB_CS ``` environment variable (which allows you 
to run the host without any command line arguments).   

If the ```iothubowner``` policy connection string is provided, it will automatically create a proxy entry in your Iot Hub using 
the host name of the machine it is running on, and then wait and listen for requests.  Use the ```-D``` option to persist the
registration information locally.  

For example, to install a proxy, you can run ```proxy -i -C iothubowner.txt -D config.db``` which will add the device to the 
config.db file, and optionally safe the key in a platform specific secure location.  You can then run ```proxy -D config.db``` 
to start the proxy.  The proxy will read the connection string from config.db, connect to Azure, and wait for requests.

Run the ```proxyd``` executable with ```--help``` command line switch to see all available options. 

A docker container that contains the proxy module host can be built and run directly from github.  If you have not done 
so already, install docker on your machine, then in a terminal or console window, run:

```
docker build -t <tag> https://github.com/Azure/iot-gateway-proxy
docker run <tag> <commandlineargs>
```

## Other samples

The following samples are included and demonstrate how to use the API:

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

