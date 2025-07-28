#!/usr/bin/bash

SELF=$0
ARG=$1
echo "Start $SELF $ARG" >&2
shift
(
   echo '<tech name="All Ships">'
   grep -m1 -ho '<ship name=\"[^"]*\"' "$@" |
   sed 's/^<ship name=\"\([^"]*\)\"$/ <item>\1<\/item>/'
   echo '</tech>'
) >"$ARG"
echo "Stop $SELF" >&2
exit 0
