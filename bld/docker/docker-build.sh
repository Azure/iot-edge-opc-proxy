#!/bin/bash
# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

set -e

if [ -n "${PROXY_REPO+1}" ]; then
    rm -rf /repo

    if git clone $PROXY_REPO /repo; then
        echo "$PROXY_REPO cloned..."
    else
        echo "Failed to clone $PROXY_REPO. Clone default repo."
        git clone https://github.com/Azure/iot-edge-opc-proxy /repo
    fi
fi
    cd repo
if [ $? -eq 0 ]; then
    echo "... repo source exists."
else
    echo "Failed to change into repo folder"
    exit $?
fi

if [ -n "${COMMIT_ID+1}" ]; then
    if git checkout -q $COMMIT_ID; then
        echo "... and sync'ed to $COMMIT_ID."
    else
        echo "Failed to checkout $COMMIT_ID."
        exit $?
    fi

    git submodule update --init
fi
    cd bld
    echo "Calling 'build.sh $*'"
    /bin/bash build.sh "$*"

if [ $? -eq 0 ]; then
    echo "Success" 
else
    echo "Failure building"
    mkdir -p /build
    cd /build
    echo "ERROR" >> build.err
    exit $?
fi
