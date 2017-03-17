# OPC-UA Client Sample

The OPC UA transport adapter is a plug in for the [OPC-Foundation reference stack](https://github.com/OPCFoundation/UA-.NETStandardLibrary)
and allows OPC UA clients using the stack to access OPC UA servers in a Proxy network.  

To run the sample, start a vanilla sample server.  E.g. you could
- Clone the OPC-UA reference stack repo using ```git clone https://github.com/OPCFoundation/UA-.NETStandardLibrary.git```,
- change into ```<stack-root>\SampleApplications\Samples\NetCoreConsoleServer```, and call...
- ```dotnet run```

Start the ```proxyd``` sample executable, then run the Client application in ```api/csharp/samples/Opc.Ua/Test``` which 
will connect and subscribe to the server time node on the server.

The first time you run the client sample, the server will reject the client certificate, since it does not know it. 
To make the server accept the client, add the rejected certificate to the list of known certificates on the server. 
If you use the sample server above, you can copy the certificate from the 
```OPC Foundation\CertificateStores\RejectedCertificates```
folder into the ```OPC Foundation\CertificateStores\UA Applications\certs``` folder, then run the client sample again.
