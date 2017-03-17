# Http Reverse Proxy Sample

This C# sample is a Simple Http Reverse Proxy that binds to localhost and allows a user to access web servers behind
the firewall through the proxy.  

The Url through which a remote server can be reached has the form: 
```
http(s)://localhost:8080|8081/<target-url-withouth-scheme>
``` 
Example:
``` 
http://localhost:8080/www.microsoft.com
``` 

To run the sample, start the ```proxyd``` host sample, then run the ```webserver``` sample.  The proxy must of course
be able to access the host name.
