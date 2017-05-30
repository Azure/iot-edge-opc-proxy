# Proxy netcat Sample (PNetcat)

This C# sample is a is a simple netcat (nc) like tool that bridges stdin/stdout with a proxy backed network stream.
This allows using it as a tunnel for SSH or just as a tool to test arbitrary socket endpoints.  Not all functionality
of the traditional Netcat tool is implemented to keep the sample as lightweight as possible, but contributions are
welcome.

## Prerequisites

Like any other sample included here, $_HUB_CS must be exported and point to the IoT Hub Owner connection string.  If run
with the -R option, $_SB_CS must be set to the Service bus relay connection string (see relay provider sample for details).

If it has not been started yet, start the ```proxyd``` in the same network as the ssh server or device you want to 
access. It is recommended to create an alias for ```dotnet pnetcat.dll``` e.g. as ```pnc``` that is globally accessible.

## Using Proxy netcat Sample as an SSH Proxy

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
To do so, use the -o option of SSH to specify pnetcat as your proxy command:

```
ssh USER@FINAL_DEST -o "ProxyCommand=pnc %h %p"
```

You can use the same option with scp and sftp.

For more advanced use cases, e.g. if you want to combine  a pnetcat bridge with existing SSH tunneling and jump host
functionality check out [https://en.wikibooks.org/wiki/OpenSSH/Cookbook/Proxies_and_Jump_Hosts](https://en.wikibooks.org/wiki/OpenSSH/Cookbook/Proxies_and_Jump_Hosts).