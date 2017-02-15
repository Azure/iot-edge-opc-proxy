This project has adopted the Microsoft Open Source Code of Conduct. For more information see the Code of Conduct FAQ or contact opencode@microsoft.com with any additional questions or comments.

# Azure IoT Field Gateway Proxy Module

This module is a stream proxy for Azure IoT written in C and intended to be hosted in an Edge Gateway built using the
[Azure IoT Gateway SDK](https://github.com/Azure/azure-iot-gateway-sdk).  

It can tunnel streams (at this point TCP based streams only) at the application level, from a cloud service via one or more local
network gateways to a server in the local network.  It uses IoT Hub Methods as protocol layer and IoT Hub device registry as a name 
service provider, and only requires port 443 (outgoing) open to Azure.

This allows cloud applications to send and receive raw buffers to and from IoT server devices that are accessible on the local
gateway network.  Using the proxy API you can quickly implement streaming command and control scenarios from a cloud application 
without providing edge protocol translation.  But please note that using the Proxy to access a device requires a data connection
(Websocket) between Gateway and Azure and precludes any edge intelligence scenarios on the exchanged data.   

Visit http://azure.com/iotdev to learn more about developing applications for Azure IoT.

# Setup and build
The Azure IoT Field Gateway Proxy module depends on several components which are included as submodules. Hence, if you did
not specify the ```--recursive``` option when using ```git clone``` to clone this repo, you need to first run ```git submodule update --init```
in a console or terminal window before continuing...

## Linux
- The Proxy module was successfully built and tested on Ubuntu 16.04 and Alpine Linux 3.5. On Ubuntu, open a terminal window, and first
install the required build dependencies: ```sudo apt-get install bash git cmake zlib1g-dev libssl-dev libcurl4-openssl-dev build-essential```
- (Optional) To build the dotnet samples and API, follow the instructions [here](https://www.microsoft.com/net/core#linuxubuntu) to
install .net Core.
- Run ```bash <repo-root>/bld/build.sh```.  After a successful build, all proxy binaries can be found under the /build/cmake/bin folder.

Run the build script with the ```--help``` option to see all configuration options available.

## Windows
The Proxy module was successfully built and tested on Windows 10 with Visual Studio 2015.
- Install [Visual Studio 2015](https://www.visualstudio.com/downloads/).
- Install CMAKE from [here](https://cmake.org/).  
- Run ```<repo-root>\bld\build.cmd```.  After a successful build, all proxy binaries can be found under the \build\cmake\bin folder.

Run the build script with the ```--help``` option to see all configuration options available.

# Azure IoT Gateway SDK compatibility
The current version of the Proxy module is targeted at the Azure IoT Gateway SDK 2016-12-16 version.

Use the following command line to clone the compatible version Azure IoT Gateway SDK, then follow the build instructions included:
```
git clone -b "2016-12-16" --recursive https://github.com/Azure/azure-iot-gateway-sdk.git
```

# Samples

All samples require an Azure IoT Hub and a Service Bus Hybrid Connection Namespace to be provisioned in Azure.  All of them also
need access to the ```iothubowner```, as well as Service Bus ```RootManageSharedAccessKey``` (manage, read, and write claims)
policy connection strings.  
These can be retrieved from the Azure portal under ```Shared access policies``` in the respective menus.  

Please note: the Service Bus Hybrid Connection Namespace connection string needs to have the name of the application instance appended
as follows:

```
<sb-root-connection-string>;Entity=<application-instance-name> 
```
The application instance name will be used as the name of the hybrid connection listener. 

If you run any of the .net 4.5 or 4.6 builds of the samples, then the hybrid connection listener is automatically created for you 
with the name <application-instance-name> should it not already exist.  
If you run the dotnet core version of the samples, **and the hybrid connection does not exist**, the sample will fail to work.
In this case, either run a .net 4.5 or .net 4.6 sample first, or manually provision the hybrid connection in the Azure portal.  When
doing so, ensure that "Requires client authentication" is checked, and a "Shared Access Policy" with the name "proxy" and 
"send" and "listen" claims is added.

For simplicity, the proxy executable (unless provided otherwise, e.g. through command line), as well as the samples, read 
connection strings from the  ```_HUB_CS ``` and  ```_SB_CS ``` environment variables respectively, .  

Note that if you use the  ```_SB_CS ``` environment variable to configure the samples, you will only be able to run one instance of the
sample at a time.  
If you want to run several samples simultaneously, you will need to provide each sample instance with its own Service Bus Hybrid 
Connection connection string containing a unique <application-instance-name>.

## Proxy Module Host Sample

While the Proxy module can be hosted by the field gateway, for demonstration purposes the ```proxyd``` executable can also be used.  
If you run it without any command line arguments, it will automatically create a proxy entry in your Iot Hub under the host name 
of the machine it is running on.  (provided the environment variable ```_HUB_CS``` is set).

Run the ```proxyd``` executable with ```--help``` command line switch to see further options. 

A docker container that contains the proxy module host can be built and run directly from github.  If you have not done so already,
install docker on your machine, then in a terminal or console window, run:

```
docker build -t <tag> https://github.com/Azure/iot-gateway-proxy
docker run -e _HUB_CS=<iothubowner-connection-string> <tag>
```

## Simple TCP/IP Client Sample

This C# sample is a Simple TCP/IP Services client that connects / sends / receives both synchronously and asynchronously through 
the proxy. 

On Windows, the Simple TCP/IP services can be installed in:
```
Programs and Features -> Turn Windows Features on or off -> [X] Simple TCPIP Services (i.e. echo, daytime, etc.)
```

Once installed, start the ```proxyd``` host sample, then run the ```simple``` sample.  

The sample accepts a host name, or defaults to the host name of the machine it is running on.  Though not recommended,
if you run the proxyd sample on a different machine than the one you installed the services on, then a inbound firewall rule
needs to be added for ports 7, 13, 17, and 19.  

## VNC Client Sample

The C# VNC sample uses the [RemoteViewing](https://www.zer7.com/software/remoteviewing) nuget package.  
It is included for demonstration purposes only.  Due to its dependency on WinForms it only works on Windows and does not
support essential Features such as keyboard and mouse. 

To run the sample, start the ```proxyd``` executable and VNC Server (included in ```api/csharp/samples/VNC/Test```
folder, then start ```RemoteViewing.Client``` WinForms App., enter the host name the VNC server runs on, and the password
of the server (e.g. ```test```), and click connect.

## OPC-UA Client Sample

The OPC UA transport adapter is a plug in for the [OPC-Foundation reference stack](https://github.com/OPCFoundation/UA-.NETStandardLibrary)
and allows OPC UA clients using the stack to access OPC UA servers in a Proxy network.  

To run the sample, start a vanilla sample server.  E.g. you could
- Clone the OPC-UA reference stack repo using ```git clone https://github.com/OPCFoundation/UA-.NETStandardLibrary.git```,
- change into ```<stack-root>\SampleApplications\Samples\NetCoreConsoleServer```, and call...
- ```dotnet run```

Start the ```proxyd``` sample executable, then run the Client application in ```api/csharp/samples/Opc.Ua/Test``` which will connect and 
subscribe to the server time node on the server.

The first time you run the client sample, the server will reject the client certificate, since it does not know it. 
To make the server accept the client, add the rejected certificate to the list of known certificates on the server. 
If you use the sample server above, you can copy the certificate from the ```OPC Foundation\CertificateStores\RejectedCertificates```
folder into the ```OPC Foundation\CertificateStores\UA Applications\certs``` folder, then run the client sample again.

# License

The Azure IoT Field Gateway Proxy module is licensed under the [MIT License](https://github.com/Azure/iot-gateway-proxy/blob/master/LICENSE).  
You can find license information for all third party dependencies [here](https://github.com/Azure/iot-gateway-proxy/blob/master/thirdpartynotice.txt). 
Note that not all of these dependencies need to be utilized, depending on your build configuration, or your choice of platform.

# Contributing

Contributions are welcome, in particular in the following areas 

- RTOS, OSX and Unix PAL ports and testing
- None IP streams, e.g. Pipe and Serial port remoting
- API bindings, e.g. node.js or Java

