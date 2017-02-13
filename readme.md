This project has adopted the Microsoft Open Source Code of Conduct. For more information see the Code of Conduct FAQ or contact opencode@microsoft.com with any additional questions or comments.

# Beta Azure IoT Gateway Proxy Module

This module is a stream proxy for Azure IoT written in C and intended to be hosted in a Edge Gateway built using the
[Azure IoT Gateway SDK](https://github.com/Azure/azure-iot-gateway-sdk).  

It can tunnel streams (at this point TCP based streams only) at the application level, from a cloud service via one or more local
network gateways to a server in the local network.  It uses IoT Hub Methods as protocol layer and IoT Hub device registry as a name 
service provider. 
This allows cloud applications to send and receive raw buffers to and from (always connected) local IoT server devices.  
Using proxy you can implement command and control scenarios that require streams or channels without implementing a complicated
protocol translation module at the edge.  

Visit http://azure.com/iotdev to learn more about developing applications for Azure IoT.


## Azure IoT Gateway SDK compatibility

Current version of the module is targeted at the Azure IoT Gateway SDK 2016-12-16 version.
Use the following script to download the compatible version Azure IoT Gateway SDK.
```
git clone -b "2016-12-16" --recursive https://github.com/Azure/azure-iot-gateway-sdk.git
```

## Setup and build

The Proxy module depends on several components which are included as submodules. Therefore, if you did not specify --recursive during
clone, you need to run 
```
git submodule update --init
```
before building.

### Windows

The module was successfully built and tested on Windows 10 with Visual Studio 2015.

- Install Visual Studio 2015.
- Install CMAKE from [here](https://cmake.org/).  
- Run <repo-root>\bld\build.cmd

The proxy binaries can be found under the \build\cmake\bin folder.

Run the build script with --help to see all configuration options available.

### Linux

- The module was successfully built and tested on Ubuntu 16.04 and Alpine Linux 3.5. On Ubuntu, install the required build dependencies using apt:
```
sudo apt-get install bash git cmake zlib1g-dev libssl-dev libcurl4-openssl-dev build-essential
```
- Optionally, to build the dotnet samples and API, .net core must be installed (i.e. following the instructions 
[here](https://www.microsoft.com/net/core#linuxubuntu)).
- Run <repo-root>/bld/build.sh in bash

The proxy binaries can be found under the /build/cmake/bin folder.

Run the build script with --help to see all configuration options available.

### Docker

A docker container that contains a standalone host (proxyd) that hosts the module can be built directly from github:
```
docker build -t <tag> https://github.com/Azure/iot-gateway-proxy
docker run -e _HUB_CS=<iothubowner-connection-string> <tag>
```

See below for more information on connection strings.

## Samples

All samples below require a Azure IoT Hub and Service Bus Namespace to be provisioned in Azure and access to
the ```iothubowner``` connections tring, and Service Bus ```root``` connection string (manage, read, and write claims).  
These can be added or retrieved from the Azure portal.  

The Service Bus connection string needs to have the name of the hybrid connection listener for the application appended
as follows:

```
<sb-root-connection-string>;Entity=<relay-name> 
```

If you run the .net 4.5 or 4.6 samples, then the hybrid connection listener is automatically created for you with the name <relay-name>
should it not already exist.  If you run the dotnet core samples, and the hybrid connection does not exist, the sample will fail to work.
In this case, either run a .net 4.5 or .net 4.6 sample first, or manually provision the hybrid connection in the Azure portal.  When
doing so, ensure that "Requires client authentication" is checked, and add a "Shared Access Policy" with the name "proxy" and 
"send" and "listen" claims.

For simplicity, the proxy executable, as well as the samples, read these connection strings from the  ```_HUB_CS ``` and  ```_SB_CS ``` 
environment variables respectively, if not otherwise provided (e.g. through command line).  

You can only run one sample at a time.  If you want to run several samples at the same time, you will need to give each its own 
Service Bus Hybrid Connection Listener in the form of a dedicated Service Bus Connection string.

### Proxy Module Host Sample

While the proxy module can be hosted by the field gateway, for demonstration purposes the proxyd executable can be used.  If you run 
it without any command line arguments, it will automatically create a proxy entry in your Iot Hub under the host name of the machine
it is running on.  (provided the environment variable ```_HUB_CS``` is set).

Run the proxyd executable with --help command line parameter to see further options. 

### Simple TCP/IP Client Sample

This C# sample is a Simple TCP/IP Services client that connects / sends / receives both synchronously and 
asynchronously through the proxy.  On Windows, the Simple TCP/IP services can be installed through 

```
Programs and Features -> Turn Windows Features on or off -> [X] Simple TCPIP Services (i.e. echo, daytime, etc.)
```

Once installed, start the proxyd sample, then run the simple sample.  

The sample accepts a host name, or defaults to the host name of the machine it is running on.  Though not recommended,
if you run the proxyd sample on a different machine than the one you installed the services on, then a inbound firewall rule
needs to be added for ports 7, 13, 17, and 19.  

### VNC Client Sample

The C# VNC sample uses the RemoteViewing nuget package.  It is included for demonstration purposes only, and due to its
dependency on WinForms works only on Windows.  Furthermore, the RemoteViewing library does not support essential Features
such as keyboard and mouse.  However, to run the sample, start the Proxy executable and VNC Server (included in ```api/csharp/samples/VNC/Test```
folder, then start RemoteViewing.Client WinForms App., enter the host name the VNC server runs on, and the password
of the server (e.g. ```test```), and click connect.

### OPC-UA Client Sample

The OPC UA transport adapter is a plug in for the [OPC-Foundation reference stack](https://github.com/OPCFoundation/UA-.NETStandardLibrary)
and allows OPC UA clients using the stack to access OPC UA servers in a Proxy network.  

To run the sample, start a vanilla sample server.  E.g.
- Clone the OPC-UA reference stack repo using ```git clone https://github.com/OPCFoundation/UA-.NETStandardLibrary.git```
- Change into ```<stack-root>\SampleApplications\Samples\NetCoreConsoleServer```
- ```dotnet run```

Start the proxyd sample executable, then run the Client application in ```api/csharp/samples/Opc.Ua/Test``` which will connect and 
subscribe to the server time node on the server.

Note that the first time you run the client, the server will reject the client certificate, since it does not know it yet. 
To make the server accept the client add the rejected certificate to the list of known certificates on the server. 
If you use the sample server above, you can copy the certificate from the ```OPC Foundation\CertificateStores\RejectedCertificates```
folder into the ```OPC Foundation\CertificateStores\UA Applications\certs``` folder, then try again.

## Contributing

Contributions are welcome, in particular in the following areas 

- RTOS, OSX and Unix PAL ports and testing
- None IP streams, e.g. Pipe and Serial port remoting
- API bindings, e.g. node.js or Java

