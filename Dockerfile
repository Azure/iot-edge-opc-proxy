FROM alpine:3.6

RUN set -ex \
        && \
    apk add --update --no-cache --virtual .build-deps \
        bash \
        cmake \
        build-base \
        gcc \
        abuild \
        binutils \
        make \
        linux-headers \
        libc-dev \
        libressl-dev \
        curl-dev \
        avahi-dev

ADD / /proxy_build
        
RUN set -ex \
        && \
	bash /proxy_build/bld/build.sh -C Release --skip-unittests --use-zlog --use-dnssd embedded \
        && \
    mv /proxy_build/build/cmake/Release/bin/* /usr/bin \
        && \
    mv /proxy_build/build/cmake/Release/libwebsockets/lib/* /usr/lib \
        && \
    mv /proxy_build/build/cmake/Release/lib/* /usr/lib \
        && \
    rm -rf /proxy_build
   
RUN set -ex \
        && \
    apk del .build-deps \
        && \
    apk add --update --no-cache --virtual .run-deps \
        bash \
        curl \
        libressl \
        ca-certificates \
        avahi

ENTRYPOINT ["proxyd"]
