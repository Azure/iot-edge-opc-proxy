# Proxy netcat Sample (PNetcat)

This C# sample is a is a simple netcat (nc) like tool that bridges stdin/stdout with a proxy backed network stream.
This allows using it as a tunnel for SSH or just as a tool to test arbitrary socket endpoints.  Not all functionality
of the traditional Netcat tool is implemented to keep the sample as lightweight as possible, but contributions are
welcome.

## Prerequisites

Like any other sample included, $_HUB_CS must be exported and point to the IoT Hub Owner connection string. If it 
has not been started yet, start the ```proxyd``` in the same network as the ssh server or device you want to access. 

## Using PNetcat as an SSH Proxy

You can ssh directly through the proxy tunnel into an SSH server using the proxy command of the SSH client as pictured
here:

```
+----------------------------------------+                           +----------------------------------------+
| +-------------+        +-------------+ |      +-------------+      | +-------------+        +-------------+ |
| |     SSH     | -----> |   PNetcat   | -----> |   Proxyd    | -----> | SSH Server  | -----> |   Endpoint  | |
| +-------------+        +-------------+ |      +-------------+      | +-------------+        +-------------+ |
|                 Client                 |                           |                 Server                 |
+----------------------------------------+                           +----------------------------------------+
```
To do so, use the -o option of SSH to specify PNetcat as your proxy command (Ensure you are in the same folder as PNetcat.dll):

```
ssh <host> -o "ProxyCommand=dotnet PNetcat.dll <host> 22"
```

or using PuttY 0.70 on Windows:

```
putty <host> -proxycmd "dotnet PNetcat.dll <host> 22"
```

For more advanced use cases, check out [https://en.wikibooks.org/wiki/OpenSSH/Cookbook/Proxies_and_Jump_Hosts](https://en.wikibooks.org/wiki/OpenSSH/Cookbook/Proxies_and_Jump_Hosts).