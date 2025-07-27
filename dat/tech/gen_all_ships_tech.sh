#!/usr/bin/bash

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
(
   echo '<tech name="All Ships">'
   grep -m1 -ho '<ship name=\"[^"]*\"' "$@" |
   sed 's/^<ship name=\"\([^"]*\)\"$/ <item>\1<\/item>/'
   echo '</tech>'
) >"$DIR"/all_ships.xml
