#!/usr/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

DST="$@"
if [ "$DST" = "" ] ; then
   DST=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../dat")
fi

pat='<?xml version="1.0" encoding="UTF-8"?>'
for i in $(grep -L "$pat" $(grep -rl -e '^<?xml\ version' $DST --include "*.xml")) ; do
   sed 's/^<?xml\ version=\(["'\'']\)1\.0\1\ encoding=\(["'\'']\)\(\(utf\)\|\(UTF\)\)-\?8\2?>$/'"$pat/" -i "$i"
done
