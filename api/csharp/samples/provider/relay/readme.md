![Microsoft Azure Relay](https://github.com/Azure/azure-relay-dotnet/blob/master/relay.png)

# Microsoft Azure Relay Provider Sample 

This sample provider plug in implements the stream service interface using a Microsoft Azure Relay as backing. 

## Setup
The sample requires a Service Bus Hybrid Connection Namespace to be set up, as well as Service Bus 
```RootManageSharedAccessKey``` (manage, read, and write claims) policy connection strings.  

For more information, see 
- [Set up a Relay](https://docs.microsoft.com/en-us/azure/service-bus-relay/relay-hybrid-connections-dotnet-get-started) 
describes how to set up a relay listener.

Please note: the Service Bus Hybrid Connection Namespace connection string needs to have the name of the application 
instance appended as follows:

```
<sb-root-connection-string>;Entity=<application-instance-name> 
```
The application instance name will be used as the name of the hybrid connection listener. 

## Using the Relay Provider
To enable the relay provider, add the provider assembly as a reference to your code, and the following lines into your code: 

``` C#
string iotHubOwnerConnectionString = <iohub-owner-connection-string>;
string relayConnectionString = <sb-root-connection-string>;
Socket.Provider = Microsoft.Azure.Devices.Proxy.RelayProvider.Create(iotHubOwnerConnectionString, relayConnectionString);
```

If you build a of .net 4.5 or 4.6 application, then the hybrid connection listener is automatically created for you 
with the name <application-instance-name> should it not already exist.  

If you add the dotnet core version of the provider sample, **and the hybrid connection does not exist**, the sample will fail to 
work. In this case, either run a .net 4.5 or .net 4.6 sample first, or manually provision the hybrid connection in the Azure 
portal.  When doing so, ensure that "Requires client authentication" is checked, and a "Shared Access Policy" with the name 
"proxy" and "send" and "listen" claims is added.

For convinience, the Relay Provider will also try to read its connection string from the ```_SB_CS ``` environment 
variable, when not provided programmatically. 

## Limitations

Note that if you store the connection string as environment variable for the service bus provider you will only be able to run 
one instance of the sample at a time.  

If you want to run several samples simultaneously, you will need to provide each sample instance with its own 
Hybrid Connection connection string containing a unique <application-instance-name>.
