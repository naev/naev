#!/bin/bash

if [ "$1" = "-h" ] || [ "$1" = "--help" ] || [ -z "$*" ] ; then cat << EOF
usage  $(basename "$0") [-r|-l] <file>..
   Checks whether its arguments contain tabs.
   Binary files are silently ignored.
   If so, the tab positions are shown and a non-zero value is returned.
   Else, returns 0.
   If -r is set, silently replaces all tabs with 3 spaces and returns 0.
   If -l is set, outputs the list of files containing tabs and returns 0.
EOF
exit 0
fi

if [ "$1" = "-r" ] || [ "$1" = "-l" ] ; then
   ARG="$1"
   shift
   readarray -t FILES <<< "$(grep -I -l $'\t' "$@")"
   if [ "$ARG" = "-r" ] ; then
      sed -i 's/'$'\t''/   /g' "${FILES[@]}"
   else
      echo "${FILES[@]}"
   fi
else
   export GREP_COLORS=$GREP_COLORS":ms=41"
   if grep -I --color=always -n -m5 $'\t' "$@" ; then
      echo "Some files contain tabs. Use $(basename "$0") -r [FILES] to replace them with 3 spaces." >&2
      exit 1
   fi
fi
