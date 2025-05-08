#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST="${SCRIPT_DIR}/../../dat/outfits/"
#DST="${SCRIPT_DIR}/../../dat/outfits/core_engine"

echo "$DST"
for i in `find ${DST} | grep '\.mvx$'` ;
   do ${SCRIPT_DIR}/mvx2xmllua.py "$i" > "${i%mvx}xml"
done
