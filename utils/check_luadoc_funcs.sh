#!/usr/bin/bash

TMP=$(mktemp)
TMP2=$(mktemp)
res=0
trap 'rm -f "$TMP" "$TMP2" ; exit $res' EXIT

for arg in "$@" ; do
   docs/lua/c2luadoc.sh "$arg" "$TMP"
   sed ':a;N;$!ba;s/\n--/\\n--/g' < "$TMP" | grep -v '\(^$\)\|\(@module\)\|\(@function\)' > "$TMP2"
   if [ -s "$TMP2" ] ; then
      echo "$arg:"
      cat "$TMP2"
      res=1
   fi
done

