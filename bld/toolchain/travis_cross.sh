#!/bin/sh

cd $(pwd)/stretch-cross/tmp/proxy_cross/build/armhf_build
sudo schroot -c stretch-amd64-sbuild -- ../../bld/toolchain/build4armhf.sh && make
