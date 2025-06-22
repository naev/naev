#!/usr/bin/bash

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage:  $(basename "$0")" >&2
   echo "  Output the list of ssys positions in the form:" >&2
   echo "  <sys_name> <x> <y> " >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/ssys")" &&
grep -ro -m 1 '^\s*<pos x=\(["'"\'"']\)-\?[0-9.]*\1 y=\1-\?[0-9.]*\1' |
sed  -e 's ^.*/  '  -e 's/^\s*//' |
sed  's/^\(.*\)\.xml:\s*<pos x=\(["'"\'"']\)\(-\?[0-9.]*\)\2 y=\2\(-\?[0-9.]*\)\2/\1 \3 \4/'
