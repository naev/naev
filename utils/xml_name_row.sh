#!/usr/bin/bash

if [ "$1" = "-h" ] || [ "$1" = "--help" ] || [ -z "$1" ]; then
   DOC=(
      "usage:  $(basename "$0") <FIELD_LIST>"
      "  Applies name to xml_name transformation to fields specified as argument,"
      "  e.g. \"1\", \"2-\", \"1,3\". Field separator is space."
   )
   ( IFS=$'\n'; echo "${DOC[*]}" ) >&2
   exit 0
fi
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

TMP=$(mktemp)
TMP2=$(mktemp)
trap 'rm -f "$TMP" "$TMP2"' EXIT
tee >(cut --complement '-d ' -f"$1" > "$TMP")   |
cut '-d ' -f"$1"                                |
tr "[:upper:]" "[:lower:]"                      |
"$SCRIPT_DIR"/xml_name.sed > "$TMP2" &&
paste '-d ' "$TMP" "$TMP2"
