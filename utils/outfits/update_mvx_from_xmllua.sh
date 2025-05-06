#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
#DST="${SCRIPT_DIR}/../../dat/outfits"
DST="${SCRIPT_DIR}/../../dat/outfits/core_engine"

echo "$DST"
for i in `grep -rl 'lua_inline' ${DST} | grep '\.xml$'` ;
   do ${SCRIPT_DIR}/xmllua2mvx.py "$i" > "${i%xml}mvx"
done
