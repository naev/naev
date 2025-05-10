#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/outfits")

echo "$DST"
for i in `grep -rl 'lua_inline' ${DST} | grep '\.xml$'` ;
   do ${SCRIPT_DIR}/xmllua2mvx.py "$i" > "${i%xml}mvx"
done
