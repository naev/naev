#!/usr/bin/bash

if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
   DOC=(
      "usage:  $(basename "$0") [ -v | -e ]"
      "  Output the graph of ssys"
      "  Vertices are in the form:"
      "  <sys_name> <x> <y>"
      "  Edges are in the form:"
      "  <src_sys_name> <dst_sys_name> [<edge_len>]"
      ""
      "  If -v is set, only vertices are output."
      "  If -e is set, only edges are output."
   )
   for i in "${DOC[@]}"; do
      echo "$i"
   done >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/ssys")" &&

if [ ! "$1" = "-e" ] ; then
   grep -ro -m 1 '^\s*<pos x=\(["'"\'"']\)-\?[0-9.]*\1 y=\1-\?[0-9.]*\1' --include="*.xml" |
   sed  -e 's ^.*/  '  -e 's/^\s*//' |
   sed  's/^\(.*\)\.xml:\s*<pos x=\(["'"\'"']\)\(-\?[0-9.]*\)\2 y=\2\(-\?[0-9.]*\)\2/\1 \3 \4/'
fi

if [ ! "$1" = "-v" ]; then
   TMP=$(mktemp)
   TMP2=$(mktemp)
   trap 'rm -f "$TMP" "$TMP2"' EXIT
   (
      echo -n "!"
      grep -ro '^\s*<\(\(jump target="[^"]*"\)\|\(hidden/>\)\|\(exitonly/>\)\)' --include="*.xml" |
      sed 's/^\([^.]*\).xml\:\s*<jump target="\([^"]*\)"/\1 \2/' |
      sed -e '/hidden/c\! hidden' -e '/exitonly/c\! exitonly' |
      tee >(cut '-d ' -f1> "$TMP") |
      cut '-d ' -f2- |
      tr "[:upper:]" "[:lower:]" |
      ../../utils/xml_name.sed > "$TMP2" &&
      paste '-d ' "$TMP" "$TMP2" ;
   ) | sed  -e "s/^/!/" -e "s/^!!//" |
   tr -d '\n' | tr '!' '\n' |
   grep -v ' exitonly$' |
   "$SCRIPT_DIR"/edge_len.sed |
   sed -f <(
      grep -rl 'tradelane' --include="*.xml" |
      sed -e 's/.xml$//' -e 's/^\(.*\)$/\/\1\/ s\/\$\/ T\//'
   ) | sed -e 's/ T T$/ tradelane/' -e 's/ T$//'
fi
