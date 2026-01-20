#!/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

name="$1"
fname="$(echo "${name// /_}" | tr [A-Z] [a-z])"

if [ -z "$fname" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage: $0 <name>" >&2
   echo "The plugin filename will be the name with spaces replaced by _ in lowercase."
   echo "Generates the directory <plugin filename> and <plugin filename>.zip."
   exit 1
fi

if [ -d "$fname" ] ; then
   read -p "directory "$fname" alread exists, remove the old one ? [Yn] " -n 1 -r
   if [[ $REPLY =~ ^[Nn]$ ]] ; then
      exit 1
   fi
   rm -fvr "$fname" >&2
   echo >&2
fi
mkdir "$fname"

(cat <<EOF
   identifier = "$name"
   name = "$fname"
   author = "plugin generator"
   version = "1.0.0"
   abstract = "plugin generated from repo status"
   license = "CC0"
   release_status = "development"
   tags = [ ]
   naev_version = ">= $(sed 's/^\([^+]*\).*/\1/' $SCRIPT_DIR/../dat/VERSION)"
   source = "local"
EOF
) | sed 's/^   //' > "$fname/plugin.toml"

readarray -t ADDED_FILES <<< "$(git diff origin/main | sed "s/^+++ b\///; t; d")"
cp --parents "${ADDED_FILES[@]}" "$fname/"

readarray -t REMOVED_FILES <<< "$(git diff origin/main | sed   \
   -e "s/^--- a\///; t nxt; d"                                 \
   -e ":nxt; N; s/\n+++ \/dev\/null$//; t; d")"

if ! [ "${#REMOVED_FILES[@]}" = 0 ] ; then
   REM_LST="$(IFS=","; echo "[${REMOVED_FILES[*]}]")"
   echo "blacklist = ${REM_LST//,/, }" >> "$fname/plugin.toml"
fi

cd "$fname"
zip -r "$fname".zip *
mv "$fname".zip ..
echo "$fname".zip

