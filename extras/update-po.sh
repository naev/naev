#!/usr/bin/bash

if [[ ! -f "naev.6" ]]; then
   echo "Please run from Naev root directory."
   exit -1
fi

set -x

echo "src/log.h" > po/POTFILES.in
find src/ -name "*.c" | sort >> po/POTFILES.in
find dat/ -name "*.lua" | sort >> po/POTFILES.in
echo "dat/commodity.xml" >> po/POTFILES.in

