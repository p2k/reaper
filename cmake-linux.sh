#!/bin/bash

if [ -e build ];then
	rm -r build
fi

mkdir build
cp *.cl *.conf build
cd build
cmake -D CMAKE_BUILD_TYPE=Release .. || exit 1
make

