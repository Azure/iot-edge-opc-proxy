FROM debian:stretch-slim
MAINTAINER Juergen Kosel <klj@softing.com>

ENV DEBIAN_FRONTEND noninteractive

# setup workdir
RUN mkdir -p /root/work/
WORKDIR /root/work/

RUN echo "deb http://ftp.de.debian.org/debian/ stretch main contrib non-free" >> /etc/apt/sources.list

# install requiered packages
RUN apt-get update && apt-get install -y git cmake cmake-data doxygen build-essential sudo libssl1.0-dev automake autoconf libtool git-svn pkg-config byacc subversion bison flex fakeroot u-boot-tools genext2fs valgrind zlib1g-dev libxml2-dev libqt4-xml libqt4-dev libqtcore4 libgupnp-1.0-dev libglib2.0-dev libgssdp-1.0-dev libsoup2.4-dev libselinux1-dev libpcre3-dev libffi-dev uuid-dev libsqlite3-dev libavahi-common-dev libavahi-client-dev nettle-dev libgmp-dev libkrb5-dev libidn11-dev libkeyutils-dev libldap2-dev


# slim down image
# This does not reduce the image size, because it ADDS another layer
#RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* /usr/share/man/?? /usr/share/man/??_*


# Add packages for armhf buid
RUN dpkg --add-architecture armhf ; apt-get update && apt-get install -y python2.7 crossbuild-essential-armhf gcc-6-base:armhf libattr1:armhf libattr1-dev:armhf libc6:armhf libc6-dev:armhf libgcc1:armhf linux-libc-dev:armhf libxml2-dev:armhf libpcap0.8-dev:armhf libcurl4-openssl-dev:armhf zlib1g-dev:armhf libgupnp-1.0-dev:armhf libglib2.0-dev:armhf libgssdp-1.0-dev:armhf libsoup2.4-dev:armhf libselinux1-dev:armhf libpcre3-dev:armhf libffi-dev:armhf uuid-dev:armhf libsqlite3-dev:armhf libavahi-common-dev:armhf libavahi-client-dev:armhf libboost-graph-parallel-dev:armhf libssl1.0-dev:armhf nettle-dev:armhf libgmp-dev:armhf libkrb5-dev:armhf libidn11-dev:armhf libkeyutils-dev:armhf libldap2-dev:armhf librtmp-dev:armhf libssh2-1-dev:armhf libgcrypt20-dev:armhf libsasl2-dev:armhf libgnutls28-dev:armhf libicu-dev:armhf
