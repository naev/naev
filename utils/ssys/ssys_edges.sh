#!/usr/bin/bash

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage:  $(basename "$0")" >&2
   echo "  Output the list of ssys jumps in the form:" >&2
   echo "  <src_sys_name> <dst_sys_name>" >&2
   exit 0
fi

TMP=$(mktemp)
TMP2=$(mktemp)
trap 'rm -f "$TMP" "$TMP2"' EXIT

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/ssys")" &&
grep -ro '^\s*<jump target="[^"]*"' --include="*.xml" |
sed 's/^\([^.]*\).xml\:\s*<jump target="\([^"]*\)"/\1 \2/' |
tee >(cut '-d ' -f1> "$TMP") |
cut '-d ' -f2- |
tr "[:upper:]" "[:lower:]" |
../../utils/xml_name.sed > "$TMP2" &&
paste '-d ' "$TMP" "$TMP2" ;
