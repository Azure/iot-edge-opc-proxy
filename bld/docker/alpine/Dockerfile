FROM alpine:3.5

ENV PROXY_REPO https://github.com/Azure/iot-gateway-proxy
ENV COMMIT_ID master

RUN \
		set -ex \
	&& \
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

COPY docker-entrypoint.sh /
ENTRYPOINT ["bash", "/docker-entrypoint.sh"]
