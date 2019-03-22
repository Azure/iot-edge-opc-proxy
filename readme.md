This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

|Branch|Status|
|------|-------------|
|master|[![Build status](https://ci.appveyor.com/api/projects/status/jsg9w13c6uqqp90w/branch/master?svg=true)](https://ci.appveyor.com/project/marcschier/iot-edge-opc-proxy/branch/master) [![Build Status](https://travis-ci.org/Azure/iot-edge-opc-proxy.svg?branch=master)](https://travis-ci.org/Azure/iot-edge-opc-proxy)|

# Azure IoT Edge OPC

Using the Azure IoT Edge OPC Reverse Proxy, client applications can connect to devices in the local gateway network and exchange transparent payloads, allowing developers to implement applications in Azure where the command and control protocol layer resides in the cloud.  

An example for where this can be used is for tunneling OPC-UA binary protocol (secure channel) from a cloud application to a machine on the factory network, or tunneling SSH to an IoT device behind a firewall.

The proxy only requires port 443 (outgoing) open to the Internet.  

# Building

Clone the repository first.  Open a terminal or command prompt and run
```
git clone --recursive https://github.com/Azure/iot-edge-opc-proxy.git
cd iot-edge-opc-proxy
``` 

You can now build and run the proxy using [docker](https://www.docker.com/get-docker):
```
docker build -t proxyd .
docker run -it proxyd -c "<*iothubownerconnectionstring>"
``` 

If you want to build the proxy on your developer machine, follow the build instructions [here](bld/readme.md).

# Running

You can configure the proxy's functionality through its command line interface.

An IoT Hub connection string can be set via the ```-c```, ```-C``` or ```-s``` command line arguments or through the ```_HUB_CS ``` environment variable (which allows you to run the proxy without any command line arguments).   

If the ```iothubowner``` connection string is used here, the proxy will automatically register with your Iot Hub.  By default it will use the host name of the machine it is running on for the registration.  You can override this name using the ```-n``` command line option.  Use the ```-D <file>``` option to persist the retrieved registration locally (otherwise it is only stored in memory, and needs to be retrieved again when the proxy restarts).  

As an example, you can create a file that contains the connection string and save it on a USB pen drive.  On the proxy machine you can then run ```proxy -i -C <usbmntpath>/iothubowner.txt -D proxy-registration.json``` which will add the device in the proxy-registration.json file, and optionally safe the retrieved shared access key for the proxy in a secure location (depending on the capabilities of your platform and the configuration of the proxy).  You can then start the proxy by passing it the registration configuration using ```proxy -D proxy-registration.json```.  The proxy will read the connection string from proxy-registration.json, connect to Azure, and wait for requests.

More options are available to configure transport and logging.  To send raw log output to your IoT Hub event hub endpoint, use the ```-T``` option.  To ensure the proxy only uses websockets to communicate with Azure IoT Hub, use the ```-w``` switch. Use the ```--proxy```, ```--proxy-user```, and ```--proxy-pwd``` options to configure your network's web proxy.  Run the ```proxyd``` executable with ```--help``` command line switch to see all available options and additional help information. 

# Running as a module (container) in Azure IoT Edge
OPC Proxy is ready to be used as a module to run in [Azure IoT Edge](https://docs.microsoft.com/en-us/azure/iot-edge) Microsoft's Intelligent Edge framework.
We recommend to take a look on the information available on the beforementioned link and use then the information provided here.

To add OPC Proxy as module to your IoT Edge deployment, you go to the Azure portal and navigate to your IoTHub and:
* Go to IoT Edge and create or select your IoT Edge device.
* Select `Set Modules`.
* Select `Add` under `Deployment Modules` and then `IoT Edge Module`.
* In the `Name` field, enter `iot-edge-opc-proxy`.
In the `Image URI` field, enter `microsoft/iot-edge-opc-proxy:1.0.4`
* Paste the following into the `Container Create Options` field and replace `<your_iothubowner_connectionstring>` with the iothubowner connectionstring of your IoTHub:

        {
            "Hostname": "prx-test",
            "Cmd": [
                "-c",
                "\"<your_iothubowner_connectionstring>\""
            ]
        }

* This [reference (here the link to the V1.37 API)](https://docs.docker.com/engine/api/v1.37/#operation/ContainerCreate) explains which `Container Create Options` exist and what the meaning of it is.
* You can adjust the command line parameters in the `Cmd` object of the IoT Edge module configuration to fit your needs. You can use all available OPC Proxy command line options.
* Leave the other settings unchanged and select `Save`.
* Back in the `Set Modules` page, select `Next`
* Add the following route in the `Specify Routes` page:

        {
           "routes":{
              "upstream":"FROM /* INTO $upstream"
           }
        }

* Select `Next`
* Select `Submit` to send your configuration down to IoT Edge
* When you have started IoT Edge on your edge device and the docker container `iot-edge-opc-proxy` is started, you can check out the log output of OPC Proxy either by
  using `docker logs -f iot-edge-opc-proxy`.

OPC Proxy usage scenario is to connect to enable connection to devices in the network it is running on. To connect to those devices you typically use hostnames. Please be aware that you can not use the hostname of the host IoT Edge and the OPC Proxy module are running running on, because IoT Edge has configured the docker internal networking in a way that it resolves the hostname to a docker network internal IP address. To connect to a socket on the host, you need to specify the IP address or a full qualified domain name of your host.


# Samples and API

The proxy API and samples can be found here:

- [.net API and samples](https://github.com/Azure/iot-edge-opc-proxy-api-csharp.git)

# Support and Contributions

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

If you are having issues compiling or using the code in this project please feel free to log an issue in the [issues section](https://github.com/Azure/iot-edge-opc-proxy/issues) of this project.

For other issues, such as Connectivity issues or problems with the portal, or issues using the Azure IoT Hub service the Microsoft Customer Support team will try and help out on a best effort basis.
To engage Microsoft support, you can create a support ticket directly from the [Azure portal](https://ms.portal.azure.com/#blade/Microsoft_Azure_Support/HelpAndSupportBlade).

# License

The Azure IoT OPC Proxy module is licensed under the [MIT License](https://github.com/Azure/iot-edge-opc-proxy/blob/master/LICENSE). You can find license information for all third party dependencies [here](https://github.com/Azure/iot-edge-opc-proxy/blob/master/thirdpartynotice.txt). Depending on your build configuration, or your choice of platform, not all of the third party dependencies need to be utilized.

Visit http://azure.com/iotdev to learn more about developing applications for Azure IoT.
