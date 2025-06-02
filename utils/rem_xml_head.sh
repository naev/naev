#!/usr/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

DST="$@"
if [ "$DST" = "" ] ; then
   DST=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../dat")
fi

for i in $(grep -rl '^<?xml\ version' $DST --include "*.xml") ; do
   sed '0,/^<?xml\ version=\(["'\'']\)1\.0\1\ encoding=\(["'\'']\)\(\(utf\)\|\(UTF\)\)-\?8\2?>$/{//d}' -i "$i"
done
