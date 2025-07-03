#!/usr/bin/bash

repeat() {
   local -i n="$1";
   shift;
   if [ ! "$n" = "0" ] ; then
      n="$((n - 1))"
      eval "$*" | repeat "$n" "$@"
   elif [ ! "$1" = "-o" ] ; then
      cat
   fi
}
repeat "$@"
