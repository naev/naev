#!/usr/bin/bash

if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
   DOC=(
      "usage:  $(basename "$0") [ -v | -e ]"
      "  Output the graph of ssys"
      "  Vertices are in the form:"
      "  <sys_name> <x> <y>"
      "  Edges are in the form:"
      "  <src_sys_name> <dst_sys_name>"
      ""
      "  If -v is set, only vertices are output."
      "  If -e is set, only edges are output."
      ""
      "  This is a faster version of ssys_graph.py with less options."
   )
   for i in "${DOC[@]}"; do
      echo "$i"
   done >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/ssys")" &&

if [ ! "$1" = "-e" ] ; then
   grep -ro -m 1 '^\s*<pos x=\(["'"\'"']\)-\?[0-9.]*\1 y=\1-\?[0-9.]*\1' |
   sed  -e 's ^.*/  '  -e 's/^\s*//' |
   sed  's/^\(.*\)\.xml:\s*<pos x=\(["'"\'"']\)\(-\?[0-9.]*\)\2 y=\2\(-\?[0-9.]*\)\2/\1 \3 \4/'
fi

if [ ! "$1" = "-v" ]; then
   TMP=$(mktemp)
   TMP2=$(mktemp)
   trap 'rm -f "$TMP" "$TMP2"' EXIT

   grep -ro '^\s*<jump target="[^"]*"' --include="*.xml" |
   sed 's/^\([^.]*\).xml\:\s*<jump target="\([^"]*\)"/\1 \2/' |
   tee >(cut '-d ' -f1> "$TMP") |
   cut '-d ' -f2- |
   tr "[:upper:]" "[:lower:]" |
   ../../utils/xml_name.sed > "$TMP2" &&
   paste '-d ' "$TMP" "$TMP2" ;
fi
