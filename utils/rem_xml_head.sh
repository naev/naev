#!/usr/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../dat")

#<?xml version='1.0' encoding='utf8'?>
#<?xml version='1.0' encoding='UTF-8'?>

for i in $(grep -rl '^<?xml\ version' $DST --include "*.xml") ; do
   sed '0,/^<?xml\ version=\(["'\'']\)1\.0\1\ encoding=\(["'\'']\)\(\(utf\)\|\(UTF\)\)-\?8\2?>$/{//d}' -i "$i"
done
