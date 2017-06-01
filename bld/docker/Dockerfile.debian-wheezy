FROM debian:wheezy

RUN \
        set -ex \
    && \
        apt-get update && apt-get install -y \
            wget \
            build-essential \
            libcurl4-openssl-dev \
            git \
            libssl-dev \
    && \
        wget https://cmake.org/files/v3.6/cmake-3.6.3.tar.gz \
    && \
        tar xzvf cmake-3.6.3.tar.gz \
    && \
        cd cmake-3.6.3 \
    && \
        ./configure \
    && \
        make && make install \
    && \
        ldconfig
