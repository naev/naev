#!/usr/bin/bash

LST=$(mktemp)
res=0
trap 'rm -f $LST ; exit $res' EXIT

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/outfits")
grep '<item>.*</item>' "$@" | sed 's/^ *<item>\(.*\)<\/item> *$/\1/' | sed 's/\&amp;/\&/' | sort -du | sed 's/^\(.*\)$/outfit name="\1"/' | grep -f - -rl "${DST}" | sort -du > "$LST"
grep "\.\(xml\)\|\(mvx\)$" "$LST" | rev | cut -b 5- | rev | uniq -d | sed 's/$/.xml/' | diff - "$LST" | grep '>' | cut -b 2-
