#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

args=("$@")
if [ "$*" = "" ] ; then
   args+=("-r")
   path=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../dat")
   args+=("$path/spob")
   args+=("$path/ssys")
fi

grep -l '^<?xml version' "${args[@]}" --include "*.xml" | while read -r i; do
   sed '0,/^<?xml\ version=\(["'\'']\)1\.0\1\ encoding=\(["'\'']\)\(\(utf\)\|\(UTF\)\)-\?8\2?>$/{//d}' -i "$i"
done
