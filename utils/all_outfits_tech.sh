#!/usr/bin/bash


# forbid '/__excl_dir__/' in path
EXCLUDED_DIRS=('bioship' 'special' 'abilities' 'astral_projection')

# forbid '<tag>__excl_tag__</tag>' in file
EXCLUDED_TAGS=('nosteal')

# forbid '__excl_nam_pattern__' in outfit name
EXCLUDED_NAM_PAT=('GUI' 'Dummy')


DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST=$(realpath --relative-to="$PWD" "$DIR"/../dat/outfits)
(
   echo -n '<tech name="All Outfits">'
   for i in $(find "$DST"/ -type 'd' |
      sed 's/$/\//' |
      grep -v -F -f <(IFS=$'\n'; echo -n "${EXCLUDED_DIRS[*]}")
   ) ; do
      FILES="$(echo "$i"/*.xml)"
      if [ ! "$FILES" = "$i"'/*.xml' ] ; then
         grep -ho '\(<outfit name=\"[^"]*\"\)\|\(<tag>[^<]*</tag>\)' "$i"/*.xml
      fi
   done |
   sed -e 's/^[^"]*"/!/' |
   tr -d $'\n' | tr '!' $'\n' |
   grep -v -f <(
      for k in "${EXCLUDED_TAGS[@]}" ; do
         echo '<tag>'"$k"'</tag>$'
      done
   ) |
   grep -v -f <(IFS=$'\n'; echo -n "${EXCLUDED_NAM_PAT[*]}") |
   sed 's/^\([^"]*\)\".*$/ <item>\1<\/item>/'
   echo '</tech>'
) >"$(realpath --relative-to="$PWD" "$DIR"/../dat/tech/all_outfits.xml)"
