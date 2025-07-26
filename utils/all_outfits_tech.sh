#!/usr/bin/bash

DIRS=("fighter_bays" "pointdefense" "weapons" "launchers" "core_engine" "core_system" "core_hull" "structure" "flow/structure" "utility" "maps" "misc" "intrinsic" "unique" "sets")
# "special"

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

"$DIR"/../dat/outfits/py/do_it.sh

(
   echo '<tech name="All Outfits">'
   for i in "${DIRS[@]}" ; do
      grep -ho '\(<outfit name=\"[^"]*\"\)\|\(<size>[^<]*</size>\)' -r "$DIR/../dat/outfits/$i" --include "*.xml" |
      sed -e 's/^[^"]*"//' -e 's/\"\.*$/\"/' -e 's/<\/size>$/!/' |
      sed -e 's/^<size>large/<s>a/' -e 's/^<size>medium/<s>b/'  -e 's/^<size>small/<s>c/' |
      tr -d $'\n' |  tr '!' $'\n' |
      sed -e 's/"<s>/<s>/' -e 's/"/<s>c"/g' |
      tr '"' $'\n' |
      sed 's/^\([^<]*\)<s>\(.*\)/\2\1/' |
      sort -s
   done |
   grep -v -F -e "GUI" -e "Dummy" |
   sed -e 's/^./ <item>/' -e 's/$/<\/item>/'
   echo '</tech>'
) > "$DIR"/../dat/tech/all_outfits.xml
