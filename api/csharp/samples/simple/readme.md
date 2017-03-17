# Simple TCP/IP Client Sample

This C# sample is a Simple TCP/IP Services client that connects / sends / receives both synchronously and asynchronously through 
the proxy. 

## Prerequisites

On Windows, the Simple TCP/IP services can be installed in:
```
Programs and Features -> Turn Windows Features on or off -> [X] Simple TCPIP Services (i.e. echo, daytime, etc.)
```

On Linux, the simple services must be enabled in inetd configuration.

## Running the sample

Once installed, start the ```proxyd``` host sample, then run the ```simple``` sample.  

The sample accepts a host name, or defaults to the host name of the machine it is running on.  Though not recommended,
if you run the proxyd sample on a different machine than the one you installed the services on, then a inbound firewall rule
needs to be added for ports 7, 13, 17, and 19.  
