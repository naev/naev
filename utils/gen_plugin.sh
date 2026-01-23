#!/bin/env bash

# Possible improvements:
#  * --from <sha_reference> (default to origin/main)
#  * also have one-way safety, useful for dev plugins.
author="plugin generator"
version="1.0.0"
ref='origin/main'

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then cat << EOF
usage  $(basename "$0") <name> [--ref <src_branch>] [--author <author>] [--version <version>]
  The plugin filename will be <name> with spaces replaced by _,
  special characters removed, and in lowercase.
  Generates the directory <filename> and <filename>.zip,
  based on the difference between current directory status and
  the reference branch.

  The reference branch can be specified with --ref, it default to origin/main.
  If a non-default value is chosen, the plugin will be labeled as "unknown mainline-safety".

  The properties author and version of the plugin can be specified
  using --author <author> and --version and <version>.
  If unspecified, <author> is \"$author\" and version is \"$version\".

usage  (documentation) $(basename "$0") -h | --help | -d | --doc
  If -h or --help is set, display this help and quit.
  If -t or --tut is set, display the tutorial.
EOF
exit; fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

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
      "--ref")
         shift
         if [ -z "$1" ] ; then echo "missing reference branch" >&2 ; exit 1 ; fi
         if git rev-parse --verify "$1" >/dev/null 2>&1 ; then
            ref="$1"
         else
            echo 'The branch "'"$1"'" does not seem to exist.'
            exit 2
         fi ;;
      "-h" | "--help")
         $0 -h
         exit ;;
      "-t" | "--tut")
         "$SCRIPT_DIR"/shlinters/smd/smd.sh -s "$SCRIPT_DIR/../docs/manual/src/plugins/plugin_generator_tutorial.md"
         exit ;;
      *)
         if [ -n "$name" ] ; then
            echo "$1: you already have given a name: $name." >&2
            echo 'If you meant "'"$1 $name"'" as a name, do:' >&2
            echo -E " > $0 \'$1 $name\'" >&2
            exit 1
         fi
         name="$1";;
   esac
   shift
done

if [ -z "$name" ] ; then
   echo "Missing argument: <name> expected." >&2
   exit 1
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

fname="$(echo "${name// /_}" | tr '[:upper:]' '[:lower:]' | tr -d '[/\|$*@&]' | sed 's/_\+/_/g')"
identifier="$(echo "$name" | tr -d -c '[:alnum:]')"

if [ -d "$fname" ] ; then
   if [ -t 0 ]; then
      read -p "directory \"$fname\" already exists, remove the old one ? [Yn] " -n 1 -r
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

git diff --name-status origin/main "$SCRIPT_DIR"/../dat/ |
grep -q \
   -v -e "^M"$'\t'"dat/.*\.xml$" \
   -v -e "^M"$'\t'"dat/outfits/bioship/generate.py$" \
   -v -e "^M"$'\t'"dat/outfits/generated/"

if ! [ "${PIPESTATUS[1]}" = 0 ] ; then
   safe="(mainline-safe)"
else
   echo "Your plugin either adds/removes files or includes modifications "
   echo "in dat/ on non-xml files -> it won't be considered as mainline-safe."
   echo "(/dat/outfits/bioship/generate.py, /dat/outfits/generated/* also allowed)"
   echo
   safe="(mainline-UNSAFE)"
fi >&2

if ! [ "$ref" = "origin/main" ] ; then
   echo "Your reference is not origin/main, so your plugin safety status will be unknown."
   echo
   safe='unknown mainline-safety'
fi >&2

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
git diff --name-status origin/main "$SCRIPT_DIR"/../dat/ |
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

