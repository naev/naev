#!/bin/env bash

author="plugin generator"
version="1.0.0"

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then cat << EOF
usage  $(basename "$0") <name> [--author <author>] [--version <version>]
  The plugin filename will be <name> with spaces replaced by _,
  special characters removed, and in lowercase.
  Generates the directory <filename> and <filename>.zip.

  The properties author and version of the plugin can be specified
  using --author <author> and --version and <version>.
  If unspecified, <author> is \"$author\" and version is \"$version\".
EOF
exit; fi

while [ -n "$*" ] ; do
   case "$1" in
      "--author")
         shift
         if [ -z "$1" ] ; then echo "missing author" >&2 ; exit 1 ; fi
         author="$1" ;;
      "--version")
         shift
         if [ -z "$1" ] ; then echo "missing version" >&2 ; exit 1 ; fi
         version="$1" ;;
      "-h" | "--help")
         $0 -h
         exit ;;
      *)
         if [ -n "$name" ] ; then
            echo "$1: you already gave a name: $name." >&2
            echo "If you meant \"$1 $name\" as a name, do:" >&2
            echo " > $0 \'$1 $name\'" >&2
            exit 1
         fi
         name="$1";;
   esac
   shift
done

if [ -z "$name" ] ; then
   "Missing argument: <name> expected." >&2
   exit 1
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

fname="$(echo "${name// /_}" | tr '[:upper:]' '[:lower:]' | tr -d '[/\|$*@&]' | sed 's/_\+/_/g')"
identifier="$(echo "$name" | tr -d -c '[:alnum:]')"

if [ -d "$fname" ] ; then
   if [ -t 0 ]; then
      read -p "directory \"$fname\" alread exists, remove the old one ? [Yn] " -n 1 -r
      if [[ $REPLY =~ ^[Nn]$ ]] ; then
         exit 1
      fi
   else
      echo "Error: \"$fname\" already exists" >&2
      exit 1
   fi
   rm -fvr "$fname" >&2
fi
mkdir "$fname"

if [ -f "$fname.zip" ] ; then
   rm -fv "$fname.zip"
fi
echo >&2

if [ "${#identifier}" -gt 25 ] ; then
   trunc="${identifier:0:25}"
   echo -e "\nWarning: identifier \"$identifier\" truncated into \"$trunc\".\n" >&2
   identifier="$trunc"
fi

git diff --name-status origin/main HEAD "$SCRIPT_DIR"/../dat/ |
grep -q \
   -v -e "^M"$'\t'"/dat/.*\.xml$" \
   -v -e "^M"$'\t'"/dat/outfits/bioship/generate.py$"
if ! [ "$?" = "0" ] ; then
   safe="(mainline-safe)"
else
   echo "Your plugin either adds/removes files or includes modifications " >&2
   echo "in dat/ on non-xml files -> it won't be considered as mainline-safe." >&2
   safe="(mainline-UNSAFE)"
fi

(cat <<EOF
   identifier = "$identifier"
   name = "$name"
   author = "plugin generator"
   version = "1.0.0"
   abstract = "plugin generated from repo status"
   description = "$safe"
   license = "GPLv3+"
   release_status = "development"
   tags = [ ]
   naev_version = ">= $(sed 's/^\([^+]*\).*/\1/' "$SCRIPT_DIR"/../dat/VERSION)"
   source = "local"
EOF
) | sed 's/^   //' > "$fname/plugin.toml"

CHANGES=$(mktemp)
trap 'rm "$CHANGES"' exit
git diff --name-status origin/main HEAD "$SCRIPT_DIR"/../dat/ |
sed 's/^R[0-9]*\t\([^\t]\+\)\t\([^\t]\+\)/D\t\1\nA\t\2/' > "$CHANGES"

readarray -t ADDED_FILES <<< "$(sed "s/^[AM]\t//; t; d" "$CHANGES")"
if [ -n "${ADDED_FILES[*]}" ] ; then
   cp --parents "${ADDED_FILES[@]}" "$fname/"
fi

readarray -t REMOVED_FILES <<< "$(sed "s/^D\t//; t; d" "$CHANGES")"
if [ -n "${REMOVED_FILES[*]}" ] ; then
   REM_LST="$(IFS=","; echo "[${REMOVED_FILES[*]}]")"
   echo "blacklist = ${REM_LST//,/, }" >> "$fname/plugin.toml"
fi

cd "$fname" || exit
zip -r "$fname".zip -- *
mv "$fname".zip ..
echo "$fname".zip

