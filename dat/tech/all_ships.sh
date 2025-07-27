#!/usr/bin/bash


# forbid '<tag>__excl_tag__</tag>' in file
EXCLUDED_TAGS=('station')

# forbid '__excl_nam_pattern__' in outfit name
EXCLUDED_NAM_PAT=('Astral')

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST=$(realpath --relative-to="$PWD" "$DIR"/../ships)
(
   echo -n '<tech name="All Ships">'

   grep -rho '\(<ship name=\"[^"]*\"\)\|\(<tag>[^<]*</tag>\)' "$DST"|
   sed -e 's/^[^"]*"/!/' |
   tr -d $'\n' | tr '!' $'\n' |
   grep -v -f <(
      for k in "${EXCLUDED_TAGS[@]}" ; do
         echo '<tag>'"$k"'</tag>$'
      done
   ) |
   grep -v -f <(IFS=$'\n'; echo -n "${EXCLUDED_NAM_PAT[*]}") |
   sed 's/^\([^"]*\)\".*$/ <item>\1<\/item>/'

   echo
   echo '</tech>'
) >"$(realpath --relative-to="$PWD" "$DIR"/all_ships.xml)"
