#!/bin/bash

ARCH="i386;x86_64"
SDK=/Developer/SDKs/MacOSX10.6.sdk

if [ -e build ];then
	rm -r build
fi

mkdir build
cp *.cl *.conf build
cd build
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_OSX_ARCHITECTURES=$ARCH -D CMAKE_OSX_DEPLOYMENT_TARGET=10.5 -D CMAKE_OSX_SYSROOT=$SDK .. || exit 1
make

