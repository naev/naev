#!/usr/bin/bash

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
(
   echo  '<tech name="All Outfits">'
   grep -m1 -ho '<outfit name=\"[^"]*\"' "$@" |
   sed 's/^<outfit name=\"\([^"]*\)\"$/ <item>\1<\/item>/'
   echo '</tech>'
) >"$DIR"/all_outfits.xml
