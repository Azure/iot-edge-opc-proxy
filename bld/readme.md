
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

### Linux Cross Build

Debian 9 (stretch) provides tool chains for cross builds of Debian packages, as described in [CrossCompiling](https://wiki.debian.org/CrossCompiling). This tool chain could also used to cross build the proxy gateway:

First modify the script bld/toolchain/create_cross_chroot.sh to your needs. Especially you may want to specify another location for the chroot environment.

Then run the script:
```
sudo bld/toolchain/create_cross_chroot.sh
```

Then you need to make the sources of this gateway proxy available in the chroot. E.g. by mounting your home directory in the chroot, as described in [CrossCompiling](https://wiki.debian.org/CrossCompiling#Making_your_home_dir_available_in_the_chroot).

Finally create a directory for the cross build (e.g. build/armhf_build) and build:

```
mkdir armhf_build
cd armhf_build/
schroot -c stretch-amd64-sbuild -- ../../bld/toolchain/build4armhf.sh
schroot -c stretch-amd64-sbuild -- make
```

### Build and debug in WSL or docker with Visual Studio 2017

Install [Linux Workload using the Visual Studio 2017 Installer] (https://blogs.msdn.microsoft.com/vcblog/2016/03/30/visual-c-for-linux-development/).

Install all dependencies in your docker image or [WSL a.k.a bash for Windows] (https://blogs.msdn.microsoft.com/vcblog/2017/02/08/targeting-windows-subsystem-for-linux-from-visual-studio/), 
enable password authentication in ssh configuration and start ssh server for your build and debug transport:

```
sudo apt install -y openssh-server gdbserver build-essentials
sudo nano /etc/ssh/sshd_config
sudo ssh-keygen -A
sudo service ssh start
```

Install all build dependencies as you would normally do:

```
sudo apt install -y build-essentials libwebsockets-dev libcurl4-openssl-dev libssl-dev
```

Copy headers to Windows machine for Intellisense:
```
mkdir bld\remote
cd bld\remote
pscp -r <login>@localhost:/usr/include .
```

Open proxy-linux.sln in Visual Studio and build/deploy/debug.

## Windows

The Proxy module was successfully built and tested on Windows 10 with Visual Studio 2017.
- Install [Visual Studio 2017](https://www.visualstudio.com/downloads/).
- Install CMAKE (3.7 or later) from [here](https://cmake.org/).  
- Run ```<repo-root>\bld\build.cmd```.  After a successful build, all proxy binaries can be found under the \build\cmake\<platform>\bin folder.

> To build the proxy for Windows 7 (Experimental), you must install OpenSSL, and run the build command with the libwebsockets option (```--use-lws```).

# Build tool CLI

Use ```-c``` to build clean. To speed up compilation, use the ```--skip-unittests``` option.  To use zlog as logging library, specifiy the  ```--use-zlog``` command line option to the build script.  Run the build script with the ```--help``` option to see all configuration options available.

