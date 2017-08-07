# Proxy netcat Sample (PNetcat)

This C# sample is a is a simple netcat (nc) like tool that bridges a port (server mode) or stdin/stdout (client mode) with a proxy backed network stream.
This allows using it as a tunnel for SSH or just as a tool to test arbitrary socket endpoints.  

Not all functionality of the traditional Netcat tool is implemented to keep the sample as lightweight as possible, but contributions are welcome.

## Prerequisites

Like any other sample included, $_HUB_CS must be exported and point to the IoT Hub Owner connection string. If it 
has not been started yet, start the ```proxyd``` in the same network as the ssh server or device you want to access. 

## Using PNetcat as Port bridge

When you start PNetCat in server mode it acts as a local port bridge (Tunnel). 
This allows you to use any client (e.g. RDP, VNC, Modebus, etc.) to access resources on the Proxy Network.  
An example is RDP, as pictured below:

```
+----------------------------------------+                           +----------------------------------------+
| +-------------+ 44440  +-------------+ |      +-------------+ 3389 | +--------------+                       |
| | RDP client  | -----> |   PNetcat   | -----> |   Proxyd    | -----> | RDP Session  |                       |
| +-------------+        +-------------+ |      +-------------+      | +--------------+                       |
|                 Client                 |                           |                 <host>                 |
+----------------------------------------+                           +----------------------------------------+
```
To use an RDP client to access a desktop in the proxy network, start PNetCat.dll in server mode like so:

```
dotnet PNetcat.dll --local 44440 <host> 3389
```

Then connect to "localhost:44440" using the RDP client.

## Using PNetcat as an SSH Proxy

You can also ssh directly through the proxy tunnel into an SSH server as pictured here:

```
+----------------------------------------+                           +----------------------------------------+
| +-------------+  std   +-------------+ |      +-------------+  22  | +-------------+        +-------------+ |
| |     SSH     | -----> |   PNetcat   | -----> |   Proxyd    | -----> | SSH Server  | -----> |   Endpoint  | |
| +-------------+        +-------------+ |      +-------------+      | +-------------+        +-------------+ |
|                 Client                 |                           |                 <host>                 |
+----------------------------------------+                           +----------------------------------------+
```
To do so, use the -o option of SSH to direct it to use PNetcat as proxy client command 
(Ensure you are in the same folder as PNetcat.dll):

```
ssh <host> -o "ProxyCommand=dotnet PNetcat.dll <host> 22"
```

or using PuttY 0.70 on Windows:

```
putty <host> -proxycmd "dotnet PNetcat.dll <host> 22"
```

For more advanced use cases, check out [https://en.wikibooks.org/wiki/OpenSSH/Cookbook/Proxies_and_Jump_Hosts](https://en.wikibooks.org/wiki/OpenSSH/Cookbook/Proxies_and_Jump_Hosts).