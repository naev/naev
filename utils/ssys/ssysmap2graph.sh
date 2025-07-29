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
   ( IFS=$'\n'; echo "${DOC[*]}" ) >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/ssys")" &&

if [ ! "$1" = "-e" ] ; then
   join -a1 <(
      grep -ro -m 1 '^\s*<pos x=\(["'"\'"']\)-\?[0-9.]*\1 y=\1-\?[0-9.]*\1' --include="*.xml" |
      sed -e 's ^.*/  '  -e 's/^\s*//' |
      sed 's/^\(.*\)\.xml:\s*<pos x=\"\(-\?[0-9.]*\)\" y=\"\(-\?[0-9.]*\)\"/\1 \2 \3/' |
      sed 's/\([0-9][0-9]*\.[0-9]*[1-9]\)0*\>/\1/g' |
      sort -k 1b,1
   ) <(
      cd "$SCRIPT_DIR"/../../dat/ || exit 1

      join -o 1.2 2.2 <(
            grep '<spob>' ssys/*.xml |
            sed 's/^ssys\/\([^.]*\)\.xml:\ *<spob>\([^<]*\)<\/spob>/\1 \2/' |
            ../utils/xml_name_row.sh '2-' |
            awk '{print $2,$1}' |
            sort -k 1b,1
         ) <(
            grep '<faction>' spob/*.xml |
            sed 's/^spob\/\([^.]*\)\.xml:\ *<faction>\([^<]*\)<\/faction>/\1 \2/' |
            grep -v Derelict |
            ../utils/xml_name_row.sh '2-' |
            sort -k 1b,1
         ) |
      sort | uniq -c | sed 's/^ *//' |
      sed 's/^[0-9]* \([^ ]*\) \(\(independent\)\|\(traders_society\)\)/0 \1 \2/' |
      awk '{print $2,$1,$3}' | sort -r -b |
      awk '{print $3,$1}' | uniq -f1 | awk '{print $2,$1}' | tac
   )
fi

if [ ! "$1" = "-v" ]; then
   (
      echo -n "!"
      grep -ro '^\s*<\(\(jump target="[^"]*"\)\|\(hidden/>\)\|\(exitonly/>\)\)' --include="*.xml" |
      sed 's/^\([^.]*\).xml\:\s*<jump target="\([^"]*\)"/\1 \2/' |
      sed -e '/hidden/c\! hidden' -e '/exitonly/c\! exitonly' |
      ../../utils/xml_name_row.sh '2-'
   ) |
   sed  -e "s/^/!/" -e "s/^!!//" |
   tr -d '\n' | tr '!' '\n' |
   grep -v ' exitonly$' |
   "$SCRIPT_DIR"/edge_len.sed |
   sed -f <(
      grep -rl 'tradelane' --include="*.xml" |
      sed -e 's/.xml$//' -e 's/^\(.*\)$/\/\1\/ s\/\$\/ T\//'
   ) | sed -e 's/ T T$/ tradelane/' -e 's/ T$//'
fi

if [ ! "$1" = "-v" ] && [ ! "$1" = "-e" ] ; then echo '' ; fi
