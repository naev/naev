#!/usr/bin/bash

TMP=`tempfile`
LST=`tempfile`
res=0
trap 'rm -f $TMP $LST ; exit $res' EXIT

for arg in $@ ; do
   grep '^ \* @luafunc' $arg | sort > $TMP
   sort -u $TMP | diff $TMP - | sort -u - | grep '<' | cut '-d<' -f2- > $LST

   if [ -s "$LST" ] ; then
      echo "$arg:"
      cat $LST
      res=1
   fi
done

