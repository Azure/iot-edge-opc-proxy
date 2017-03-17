# VNC Client Sample

The C# VNC sample uses the [RemoteViewing](https://www.zer7.com/software/remoteviewing) nuget package.  
It is included for demonstration purposes only.  Due to its dependency on WinForms it only works on Windows and does not
support essential Features such as keyboard and mouse. 

## Running the sample

To run the sample, follow the instructions on how to set up IoT Hub and [ServiceBus Relay](../provider/relay/readme.md) 
and configure both connection strings to be available as environment variables on your system.

Start the ```proxyd``` executable and VNC Server (included in ```api/csharp/samples/VNC/Test```
folder, then start ```RemoteViewing.Client``` WinForms App., enter the host name the VNC server runs on, and the password
of the server (e.g. ```test```), and click connect.
