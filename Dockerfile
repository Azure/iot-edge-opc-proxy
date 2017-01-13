FROM ubuntu:16.04

RUN apt-get clean && \
    apt-get update && \
    apt-get update && \
    apt-get install -y \
        curl \
        build-essential \
        libcurl4-openssl-dev \
        git \
        cmake \
        libssl-dev \
        valgrind \
        libglib2.0-dev

ENV PROXY_REPO https://github.com/Azure/iot-gateway-proxy
ENV COMMIT_ID master

ENTRYPOINT rm -rf /proxy && \
           git clone ${PROXY_REPO} /proxy && \
           git -C /proxy checkout ${COMMIT_ID} && \
           git -C /proxy submodule update --init && \
           chmod +x /proxy/bld/build.sh && \
           /proxy/bld/build.sh -rv && \
           bash
