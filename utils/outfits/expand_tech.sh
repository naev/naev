#!/bin/bash


SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/outfits")
grep '<item>.*</item>' "$@" | sed 's/^ *<item>\(.*\)<\/item> *$/\1/' | sed 's/\&amp;/\&/' | \
sort -du | sed 's/^\(.*\)$/outfit name="\1"/' | grep -f - -rl "${DST}" | grep "\.xml$" | \
sort -du
