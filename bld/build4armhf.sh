#!/bin/bash
# Script to create the makefile to cross compile for host linux-armhf
# Assuming that the variable ARMHF_SYSROOT_DIR points to the system root
# of the host system with requiered libraries and headers.

repo_root=$(cd "$(dirname "$0")/.." && pwd)

# Also assuming that the current working directory is the desired directory for the build
cmake -DCMAKE_TOOLCHAIN_FILE=${repo_root}/bld/cmake_toolchain_file_armhf_linux \
      -DOPENSSL_ROOT_DIR=$ARMHF_SYSROOT_DIR/usr \
      -DOPENSSL_INCLUDE_DIR=$ARMHF_SYSROOT_DIR/usr/include \
      -DOPENSSL_LIBRARIES="-L$ARMHF_SYSROOT_DIR/usr/lib/arm-linux-gnueabihf -lssl -lcrypto" \
      -DCMAKE_BUILD_TYPE=Debug \
      -Duse_zlog:BOOL=OFF "$repo_root"
