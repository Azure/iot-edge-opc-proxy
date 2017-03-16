# OPC-UA Client Sample

The OPC UA transport adapter is a plug in for the [OPC-Foundation reference stack](https://github.com/OPCFoundation/UA-.NETStandardLibrary)
and allows OPC UA clients using the stack to access OPC UA servers in a Proxy network.  

## Prerequisites

The sample requires an Azure IoT Hub to be provisioned in your Azure Subscription.  All of them also need access to the ```iothubowner``` policy connection string.
For more information, see

- [Set up IoT Hub](https://github.com/Azure/azure-iot-sdks/blob/master/doc/setup_iothub.md) describes how to configure your Azure IoT Hub service.
- [Manage IoT Hub](https://github.com/Azure/azure-iot-sdks/blob/master/doc/manage_iot_hub.md) describes how to provision devices in your Azure IoT Hub service.

For simplicity, the default Iot Hub provider reads the connection string from the  ```_HUB_CS ``` environment variables when not provided programmatically.  

The sample also requires a Service Bus Hybrid Connection Namespace to be set up, as well as Service Bus ```RootManageSharedAccessKey``` (manage, read, and write claims)
policy connection strings.  The Service Bus Provider plug in reads its connection string from the ```_SB_CS ``` environment variable.  
For more information, see 
- [Set up a Relay](https://docs.microsoft.com/en-us/azure/service-bus-relay/relay-hybrid-connections-dotnet-get-started) describes how to set up a relay listener.

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

Also note that if you store the connection string as environment variable for the service bus provider you will only be able to run one 
instance of the sample at a time.  If you want to run several samples simultaneously, you will need to provide each sample instance with its own Service Bus Hybrid 
Connection connection string containing a unique <application-instance-name>.

## Running the sample

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
