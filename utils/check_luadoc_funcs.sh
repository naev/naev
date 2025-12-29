#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

TMP=$(mktemp -u)
mkfifo "$TMP"
TMP2=$(mktemp)

res=0
trap 'rm -f "$TMP" "$TMP2" ; exit $res' EXIT

doxt="$("$SCRIPT_DIR"/get_doxtractor.sh)"
for arg in "$@" ; do
   "$SCRIPT_DIR/../docs/lua/src2luadoc.sh" "$doxt" "$arg" "$TMP" &
   sed ':a;N;$!ba;s/\n--/\\n--/g' < "$TMP" | grep -v '\(^$\)\|\(@module\)\|\(@function\)' > "$TMP2"
   if [ -s "$TMP2" ] ; then
      echo "$arg:"
      cat "$TMP2"
      res=1
   fi
done
