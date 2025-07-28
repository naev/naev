#!/usr/bin/env bash

DST=$1
shift
(
   echo '<tech name="All Ships">'
   grep -m1 -ho '<ship name=\"[^"]*\"' "$@" |
   sed 's/^<ship name=\"\([^"]*\)\"$/ <item>\1<\/item>/'
   echo '</tech>'
) >"$DST"
