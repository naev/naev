#!/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ -z "$1" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage: $0 <name>" >&2
   echo "The plugin filename will be the name with spaces replaced by _ in lowercase."
   echo "Generates the directory <plugin filename> and <plugin filename>.zip."
   exit 1
fi

name="$1"
fname="$(echo "${name// /_}" | tr '[:upper:]' '[:lower:]' | tr -d '[/\|$*@&]' | sed 's/_\+/_/g')"
identifier="$(echo "$name" | tr -d -c '[:alnum:]')"

if [ -d "$fname" ] ; then
   if [ -t 0 ]; then
      read -p "directory $fname alread exists, remove the old one ? [Yn] " -n 1 -r
      if [[ $REPLY =~ ^[Nn]$ ]] ; then
         exit 1
      fi
   else
      echo "Error: $fname already exists" >&2
      exit 1
   fi
   rm -fvr "$fname" >&2
   echo >&2
fi
mkdir "$fname"

if [ -f "$fname.zip" ] ; then
   rm -fv "$fname.zip"
fi

if [ "${#identifier}" -gt 25 ] ; then
   trunc="${identifier:0:25}"
   echo -e "\nWarning: identifier \"$identifier\" truncated into \"$trunc\".\n" >&2
   identifier="$trunc"
fi

(cat <<EOF
   identifier = "$identifier"
   name = "$name"
   author = "plugin generator"
   version = "1.0.0"
   abstract = "plugin generated from repo status"
   license = "CC0"
   release_status = "development"
   tags = [ ]
   naev_version = ">= $(sed 's/^\([^+]*\).*/\1/' "$SCRIPT_DIR"/../dat/VERSION)"
   source = "local"
EOF
) | sed 's/^   //' > "$fname/plugin.toml"

CHANGES=$(mktemp)
trap 'rm "$CHANGES"' exit
git diff --name-status origin/main |
sed 's/^R[0-9]*\t\([^\t]\+\)\t\([^\t]\+\)/D\t\1\nA\t\2/' > "$CHANGES"

readarray -t ADDED_FILES <<< "$(sed "s/^[AM]\t//; t; d" "$CHANGES")"
cp --parents "${ADDED_FILES[@]}" "$fname/"

readarray -t REMOVED_FILES <<< "$(sed "s/^D\t//; t; d" "$CHANGES")"
if ! [ "${#REMOVED_FILES[@]}" = 0 ] ; then
   REM_LST="$(IFS=","; echo "[${REMOVED_FILES[*]}]")"
   echo "blacklist = ${REM_LST//,/, }" >> "$fname/plugin.toml"
fi

cd "$fname" || exit
zip -r "$fname".zip -- *
mv "$fname".zip ..
echo "$fname".zip

