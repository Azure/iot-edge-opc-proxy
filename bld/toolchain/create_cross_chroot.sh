#!/bin/bash
# Script to prepare the chroot for cross compilation. Similar to the description in https://wiki.debian.org/CrossCompiling
# Run this script as root

apt-get install sbuild

# Create the sbuild chroot:
sbuild-createchroot stretch $(pwd)/stretch-cross http://ftp.de.debian.org/debian/ $(dirname "$0")/stretch
# In the line above, you probably want to specify another location for the chroot
# and another mirror (e.g. an approx packet proxy).

# sbuild will install the requiered packages to cross build debian packages, while building a debian packet.
# But the follwing will install the packages permanently in the chroot.
# Which save bandwidth and works to cross compile not debianized sources.
sbuild-apt stretch-amd64-sbuild apt-get install sudo
sbuild-apt stretch-amd64-sbuild apt-get install crossbuild-essential-armhf
sbuild-apt stretch-amd64-sbuild apt-get install git cmake
sbuild-apt stretch-amd64-sbuild apt-get install autoconf automake autopoint autotools-dev bsdmainutils debhelper dh-autoreconf dh-strip-nondeterminism gettext gettext-base groff-base intltool-debian libarchive-zip-perl libcroco3 libffi6 libfile-stripnondeterminism-perl libglib2.0-0 libncurses5 libpipeline1 libsigsegv2 libtimedate-perl libtool libunistring0 m4 man-db po-debconf libbsd0
schroot -c source:stretch-amd64-sbuild -- dpkg --add-architecture armhf
sbuild-apt stretch-amd64-sbuild apt-get update
sbuild-apt stretch-amd64-sbuild apt-get install gcc-6-base:armhf libattr1:armhf libattr1-dev:armhf libc6:armhf libc6-dev:armhf libgcc1:armhf linux-libc-dev:armhf
sbuild-apt stretch-amd64-sbuild apt-get install libxml2-dev:armhf libpcap0.8-dev:armhf libcurl4-openssl-dev:armhf zlib1g-dev:armhf libgupnp-1.0-dev:armhf libglib2.0-dev:armhf libgssdp-1.0-dev:armhf libsoup2.4-dev:armhf libselinux1-dev:armhf libpcre3-dev:armhf libffi-dev:armhf uuid-dev:armhf libsqlite3-dev:armhf libavahi-common-dev:armhf libavahi-client-dev:armhf libboost-graph-parallel-dev:armhf libssl-dev:armhf
sbuild-apt stretch-amd64-sbuild apt-get install nettle-dev:armhf libgmp-dev:armhf libkrb5-dev:armhf libidn11-dev:armhf libkeyutils-dev:armhf libldap2-dev:armhf librtmp-dev:armhf libssh2-1-dev:armhf libgcrypt20-dev:armhf libsasl2-dev:armhf libgnutls28-dev:armhf
