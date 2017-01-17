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
        
RUN chmod +x /proxy_build/bld/build.sh \
        && \
    /proxy_build/bld/build.sh --skip-unittests --use-zlog \
        && \
    mv /proxy_build/build/bin/* /usr/bin \
        && \
    mv /proxy_build/build/libwebsockets/lib/* /usr/lib \
        && \
    mv /proxy_build/build/lib/* /usr/lib \
            && \
    rm -rf /proxy_build
   
RUN apk del .build-deps \
        && \
    apk add --no-cache --virtual .run-deps \
        bash \
        curl

ENTRYPOINT \
    proxyd
