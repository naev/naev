#!/usr/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

args=("$@")
if [ "$*" = "" ] ; then
   args+=("$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../dat")")
fi

grep -rL '^<?xml\ version' "${args[@]}" --include "*.xml" | while read -r i; do
   sed -i '1 i\<?xml version="1.0" encoding="UTF-8"?>' "$i"
done
