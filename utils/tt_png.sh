#!/usr/bin/env bash

BAS_EXPR='2 / 3'
BAS_SUFF='_tt'
ALT='_r'

if [ "$1" = "-r" ] ; then
   REM=1
   shift
fi

EXPR="$BAS_EXPR"
SUFF="$BAS_SUFF"
if [ "$1" = "-e" ] ; then
   shift
   if [ -n "$1" ] && [ ! "${1:1:1}" = "-" ] ; then
      EXPR="$1"
      SUFF="$ALT"
      shift
   fi
fi

if [ -z "$1" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   DOC=(
      "usage:  $(basename "$0") [-r] [-e <expr>] <file1.png> .."
      "  For each file <name.png> in arg list, creates <name_tt.png>"
      "  that is a two thirds resize of <name.png>."
      "  If -e is set, applies * <expr> instead of * $BAS_EXPR. The suffix then"
      "  becomes $ALT instead of $BAS_SUFF."
      "  If -r is set, <name.png> is removed."
   )
   ( IFS=$'\n'; echo "${DOC[*]}" ) >&2
   exit 1
fi

for line in "$@" ; do
   res=$(identify -verbose "$line" | grep -m 1 'geometry:')
   if [ "$res" != "" ] ; then
      # shellcheck disable=SC2001
      geom=$(sed "s/^.*geometry: \([0-9]*x[0-9]*\).*$/\1/" <<< "$res")
      W="$(echo "$geom" | cut -dx -f1)"
      H="$(echo "$geom" | cut -dx -f2)"
      # shellcheck disable=SC2004
      newgeom="$(( W * $EXPR ))x$(( H * $EXPR ))"
      new="${line%.png}$SUFF.png"
      echo -n "$line:$geom -> $new:$newgeom "
      convert -scale "$newgeom" "$line" "$new"
      if [ -n "$REM" ] ; then
         rm -v "$line"
      else
         echo
      fi
   fi
done >&2
