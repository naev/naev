#!/usr/bin/bash

TMP=`tempfile`
LST=`tempfile`
trap 'rm -f $TMP $LST' EXIT

grep '^ \* @luafunc' $1 | sort > $TMP
sort -u $TMP | diff $TMP - | sort -u - | grep '<' | cut '-d<' -f2- > $LST

if [ -s "$LST" ] ; then
   echo "$1:"
   cat $LST
   exit 1
fi
