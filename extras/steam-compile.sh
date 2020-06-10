#!/bin/bash

make distclean
./autogen.sh
./configure --enable-lua=internal --without-libzip
make -j 5

