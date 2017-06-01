
# Build

> The Azure IoT Edge OPC Proxy module depends on several components which are included as submodules. Hence, if you did not specify the ```--recursive``` option when you cloned the repo, you need to first run ```git submodule update --init``` in a console or terminal window before continuing.

## Linux
The Proxy module was tested on Ubuntu 16.04 and Alpine Linux 3.5, but can be built on a variety of Linux flavors. Check out the [Dockerfile folder](/docker) to find examples on how to set up your specific distribution.  If you do not find yours in the folder, consider contributing a Dockerfile for your distribution to this project.

To build the dotnet samples and API, follow the instructions [here](https://www.microsoft.com/net/core#linuxubuntu) to install .net Core.
Run ```bash <repo-root>/bld/build.sh```.  After a successful build, all proxy binaries can be found under the /build/cmake/bin folder.

### Install (Optional)
To install run the usual ```make install``` in directory build/cmake/Release or build/cmake/Debug. For a test 
install to /tmp/azure you could use for example the following: ```make -C <repo-root>/build/cmake/Debug  DESTDIR=/tmp/azure 
install```.  Then run the proxy: ```LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/tmp/azure/usr/local/lib /tmp/azure/usr/local/bin/proxyd 
--help```

## Windows
The Proxy module was successfully built and tested on Windows 10 with Visual Studio 2017.
- Install [Visual Studio 2017](https://www.visualstudio.com/downloads/).
- Install CMAKE (3.7 or later) from [here](https://cmake.org/).  
- Run ```<repo-root>\bld\build.cmd```.  After a successful build, all proxy binaries can be found under the \build\cmake\<platform>\bin folder.

> To build the proxy for Windows 7 (Experimental), you must install OpenSSL, and run the build command with the libwebsockets option (```--use-lws```).

# Build tool CLI

Use ```-c``` to build clean. To speed up compilation, use the ```--skip-unittests``` option.  To use zlog as logging library, specifiy the  ```--use-zlog``` command line option to the build script.  Run the build script with the ```--help``` option to see all configuration options available.

