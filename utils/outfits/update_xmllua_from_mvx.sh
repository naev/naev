#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/outfits")

echo "$DST"
for i in `find ${DST} | grep '\.mvx$'` ;
   do ${SCRIPT_DIR}/mvx2xmllua.py "$i" > "${i%mvx}xml"
done
