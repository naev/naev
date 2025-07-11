#!/usr/bin/bash

if [ "$1" = "-h" ] || [ -z "$1" ] || [ "$1" = "--help" ] ; then
   DOC=(
      "This is a bash helper func. Takes:"
      "- a number >= 0"
      "- the commandlibne to repipe-repeat"
   )
   ( IFS=$'\n'; echo "${DOC[*]}" ) >&2
   exit 1
fi
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
