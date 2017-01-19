FROM alpine:3.5

RUN set -ex && \
    apk add --no-cache --virtual .fetch-deps \
        git \
        bash \
    && \
    apk add --no-cache --virtual .build-deps \
        cmake \
        build-base \
        gcc \
        abuild \
        binutils \
        make \
        libc-dev \
        curl-dev

ENV PROXY_REPO https://github.com/Azure/iot-gateway-proxy
ENV COMMIT_ID master

ENTRYPOINT \
    rm -rf /proxy && \
    git clone ${PROXY_REPO} /proxy && \
    git -C /proxy checkout ${COMMIT_ID} && \
    git -C /proxy submodule update --init && \
    chmod +x /proxy/bld/build.sh && \
    /proxy/bld/build.sh --skip-unittests --use-zlog && \
    bash
