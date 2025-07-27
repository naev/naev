#!/usr/bin/bash


# forbid '<tag>__excl_tag__</tag>' in file
EXCLUDED_TAGS=('station')

# forbid '__excl_nam_pattern__' in outfit name
EXCLUDED_FIL_PAT=('astral')

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

grep -rL -F --include="*.xml" -f <(
   for k in "${EXCLUDED_TAGS[@]}" ; do
      echo '<tag>'"$k"'</tag>'
   done
) "$(realpath --relative-to="$PWD" "$DIR"/../ships)" |
grep -v -f <(IFS=$'\n'; echo -n "${EXCLUDED_FIL_PAT[*]}")
