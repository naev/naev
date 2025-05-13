#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/outfits")

echo Generate from "$DST" >&2
for i in `grep -rl 'lua_inline' ${DST} | grep '\.xml$'` ; do
   ${SCRIPT_DIR}/xmllua2mvx.py "$i" > $(dirname "$i")/.$(basename "$i" xml)mvx
done
