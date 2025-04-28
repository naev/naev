#!/usr/bin/bash

LST=$(mktemp)
res=0
trap 'rm -f $LST ; exit $res' EXIT

for arg in "$@" ; do
   grep '^ \* @luafunc' "$arg" | sort | uniq -c | grep -v "^ *1 " > "$LST"

   if [ -s "$LST" ] ; then
      echo "$arg:"
      cat "$LST"
      res=1
   fi
done

