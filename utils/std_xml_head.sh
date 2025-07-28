#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

args=("$@")
if [ "$*" = "" ] ; then
   args+=("-r")
   args+=("$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../dat")")
fi

pat='<?xml version="1.0" encoding="UTF-8"?>'
grep -L "$pat" "$(grep -rl -e '^<?xml\ version' "${args[@]}" --include "*.xml")" | while read -r i; do
   sed '1s/^<?xml\ version=\(["'\'']\)1\.0\1\ encoding=\(["'\'']\)[uU][tT][fF]-\?8\2?>$/'"$pat"'/' -i "$i"
done
