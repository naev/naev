#!/usr/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../dat")

for i in $(grep -rL '^<?xml\ version' $DST --include "*.xml") ; do
   sed -i '1 i\<?xml version="1.0" encoding="UTF-8"?>' "$i"
done
