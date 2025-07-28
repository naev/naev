#!/usr/bin/bash

SELF=$0
ARG=$1
echo "Start $SELF $ARG" >&2
shift
(
   echo  '<tech name="All Outfits">'
   grep -m1 -ho '<outfit name=\"[^"]*\"' "$@" |
   sed 's/^<outfit name=\"\([^"]*\)\"$/ <item>\1<\/item>/'
   echo '</tech>'
) >"$ARG"
echo "Stop $SELF" >&2
exit 0
