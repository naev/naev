#!/usr/bin/env bash


# forbid '/__excl_dir__/' in path
EXCLUDED_DIRS=('bioship' 'special' 'abilities' 'astral_projection')

# forbid '<tag>__excl_tag__</tag>' in file
EXCLUDED_TAGS=('nosteal')

# forbid '__excl_filnam_pattern__' in outfit filename
EXCLUDED_FIL_PAT=('gui-' 'dummy')


for i in $(find "$1" -type 'd' |
   sed 's/$/\//' |
   grep -vF -f <(IFS=$'\n'; echo -n "${EXCLUDED_DIRS[*]}")
) ; do
   FILES=("$i"*.xml)
   if [ ! "${FILES[0]}" = "$i"'*.xml' ] ; then
      grep -LF -f <(
         for k in "${EXCLUDED_TAGS[@]}" ; do
            echo '<tag>'"$k"'</tag>'
         done
      ) "${FILES[@]}"
   fi
done |
grep -v -f <(IFS=$'\n'; echo -n "${EXCLUDED_FIL_PAT[*]}")
