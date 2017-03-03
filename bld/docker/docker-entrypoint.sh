#!/bin/bash
# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

set -e
rm -rf /proxy

if git clone $PROXY_REPO /proxy; then
	echo "$PROXY_REPO cloned..."
else
	echo "Failed to clone $PROXY_REPO. Clone default repo."
	git clone https://github.com/Azure/iot-gateway-proxy /proxy
fi
cd proxy
if git checkout -q $COMMIT_ID; then
	echo "... and sync'ed to $COMMIT_ID."
else
	echo "Failed to checkout $COMMIT_ID."
fi

git submodule update --init
cd bld

echo "Calling 'build.sh $*'"
chmod +x build.sh

	( ./build.sh "$*" )

if [ $? -eq 0 ]; then
	echo "Success" 
else
	echo "Failure building"
	mkdir -p /build
	cd /build
	echo "ERROR" >> build.err
	exit $?
fi
