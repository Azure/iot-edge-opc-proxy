FROM alpine:3.5

RUN set -ex \
        && \
    apk add --no-cache --virtual .build-deps \
        bash \
        cmake \
        build-base \
        gcc \
        abuild \
        binutils \
        make \
        libc-dev \
        curl-dev

ADD / /proxy_build
        
RUN \
	bash /proxy_build/bld/build.sh -C Release --skip-unittests --use-zlog \
        && \
    mv /proxy_build/build/cmake/Release/bin/* /usr/bin \
        && \
    mv /proxy_build/build/cmake/Release/libwebsockets/lib/* /usr/lib \
        && \
    mv /proxy_build/build/cmake/Release/lib/* /usr/lib \
            && \
    rm -rf /proxy_build
   
RUN apk del .build-deps \
        && \
    apk add --no-cache --virtual .run-deps \
        bash \
        curl

ENTRYPOINT \
    proxyd
