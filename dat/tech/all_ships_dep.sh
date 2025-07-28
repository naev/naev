#!/bin/bash


# forbid '<tag>__excl_tag__</tag>' in file
EXCLUDED_TAGS=('station')

# forbid '__excl_nam_pattern__' in outfit name
EXCLUDED_FIL_PAT=('astral')

echo "Start $0 $1" >&2
grep -rL -F --include="*.xml" -f <(
   for k in "${EXCLUDED_TAGS[@]}" ; do
      echo '<tag>'"$k"'</tag>'
   done
) "$1" |
grep -v -f <(IFS=$'\n'; echo -n "${EXCLUDED_FIL_PAT[*]}")
echo "Stop $0" >&2
exit 0
