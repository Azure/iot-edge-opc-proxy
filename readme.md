This project has adopted the Microsoft Open Source Code of Conduct. For more information see the Code of Conduct FAQ or contact opencode@microsoft.com with any additional questions or comments.

|Branch|Status|
|------|-------------|
|master|[![Build status](https://ci.appveyor.com/api/projects/status/do87bhdyyykf6sbj/branch/master?svg=true)](https://ci.appveyor.com/project/marcschier/iot-gateway-opc-ua-proxy/branch/master) [![Build Status](https://travis-ci.org/Azure/iot-edge-opc-proxy.svg?branch=master)](https://travis-ci.org/Azure/iot-edge-opc-proxy)|

# Azure IoT Edge OPC

Using the Azure IoT Edge OPC Reverse Proxy, client applications can connect to devices in the local gateway network and exchange transparent payloads, allowing developers to implement applications in Azure where the command and control protocol layer resides in the cloud.  

An example for where this can be used is for tunneling OPC-UA binary protocol (secure channel) from a cloud application to a machine on the factory network, or tunneling SSH to an IoT device behind a firewall.

The proxy only requires port 443 (outgoing) open to the Internet.  

# Getting started

> Before you run the proxy or any of the included samples, you must obtain a service manage *connection string* for your IoT Hub. You can use the *iothubowner* connection string (going forward referred to as **<*iothubownerconnectionstring>**) which can easily be found on the [Azure portal](https://portal.azure.com) in the "Shared Access Policies" section of the [IoT hub settings blade](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-create-through-portal#change-the-settings-of-the-iot-hub). For more information checkout out the documentation [here](https://github.com/Azure/azure-iot-device-ecosystem/blob/master/setup_iothub.md).

The proxy gateway can be built and run directly from github using [docker](https://www.docker.com/get-docker):
```
docker build -t proxyd https://github.com/Azure/iot-edge-opc-proxy.git
docker run -it proxyd -c "<*iothubownerconnectionstring>"
``` 

If you want to build the proxy on your developer machine, follow the build instructions [here](/bld/readme.md).

# Client Samples

> For simplicity, all samples read the <*iothubownerconnectionstring> from the  ```_HUB_CS ``` environment variable.  If you do not set this variable on your machine the included samples will not work!  However, make sure you safeguard the connection string properly on your development machine and do not develop against a production IoT Hub.    

## C# samples

The following samples are included to demonstrate the use of the .net API:

- An [OPC UA client](/api/csharp/samples/opc-ua/readme.md) that shows how the [OPC-Foundation reference stack](https://github.com/OPCFoundation/UA-.NETStandardLibrary) can be used to relay OPC UA from the cloud to machines in a local gateway network. 

- PNetCat - A [Netcat like proxy client](/api/csharp/samples/netcat/readme.md) that you can use to interact
with an arbitrary host and port in the proxy network and for example tunnel SSH using the ssh proxy option.

- A [Simple TCP/IP services client](/api/csharp/samples/simple/tcp/readme.md) that demonstrates different socket API calls.

- A [Browser client](/api/csharp/samples/simple/dns/readme.md) that uses the remote browsing capabilities of the proxy.  Use it to enumerate services and resolve host names across all proxies.

- A simple [Reverse HTTP Web Proxy](/api/csharp/samples/http/readme.md) that shows how you can access a web server in the proxy network through a reverse Web proxy Server implemented using ASP.Net Core and Kestrel. 

# Proxy CLI 

You can configure the proxy's functionality through its command line interface.

An IoT Hub connection string can be set via the ```-c```, ```-C``` or ```-s``` command line arguments or through the ```_HUB_CS ``` environment variable (which allows you to run the proxy without any command line arguments).   

If the ```iothubowner``` connection string is used, the proxy will automatically register with your Iot Hub.  By default it will use the host name of the machine it is running on for the registration.  You can override this name using the ```-n``` command line option.  Use the ```-D <file>``` option to persist the retrieved registration locally (otherwise it is only stored in memory, and needs to be retrieved again when the proxy restarts).  

As an example, you can create a file that contains the connection string and save it on a USB pen drive.  On the proxy machine you can then run ```proxy -i -C <usbmntpath>/iothubowner.txt -D proxy-registration.json``` which will add the device in the proxy-registration.json file, and optionally safe the retrieved shared access key for the proxy in a secure location (depending on the capabilities of your platform and the configuration of the proxy).  You can then start the proxy by passing it the registration configuration using ```proxy -D proxy-registration.json```.  The proxy will read the connection string from proxy-registration.json, connect to Azure, and wait for requests.

More options are available to configure transport and logging.  To send raw log output to your IoT Hub event hub endpoint, use the ```-T``` option.  To ensure the proxy only uses websockets to communicate with Azure IoT Hub, use the ```-w``` switch. Use the ```--proxy```, ```--proxy-user```, and ```--proxy-pwd``` options to configure your network's web proxy.  Run the ```proxyd``` executable with ```--help``` command line switch to see all available options and additional help information. 

# Support and Contributions

If you are having issues compiling or using the code in this project please feel free to log an issue in the [issues section](https://github.com/Azure/iot-edge-opc-proxy/issues) of this project.

For other issues, such as Connectivity issues or problems with the portal, or issues using the Azure IoT Hub service the Microsoft Customer Support team will try and help out on a best effort basis.
To engage Microsoft support, you can create a support ticket directly from the [Azure portal](https://ms.portal.azure.com/#blade/Microsoft_Azure_Support/HelpAndSupportBlade).

Contributions are welcome. If you would like to contribute features or fixes, please open a Pull Request.  Bonus points for contributing an integration test and/or unit test with your code. For ideas on what to work on, check out all items marked with "Help wanted" in the [issues section](https://github.com/Azure/iot-edge-opc-proxy/issues).

# License

The Azure IoT OPC Proxy module is licensed under the [MIT License](https://github.com/Azure/iot-edge-opc-proxy/blob/master/LICENSE). You can find license information for all third party dependencies [here](https://github.com/Azure/iot-edge-opc-proxy/blob/master/thirdpartynotice.txt). Depending on your build configuration, or your choice of platform, not all of the third party dependencies need to be utilized.

Visit http://azure.com/iotdev to learn more about developing applications for Azure IoT.
