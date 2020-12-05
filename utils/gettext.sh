#!/bin/sh

set -ex

if [ -z "$1" ]; then
   BASEPATH="."
else
   BASEPATH="$1"
fi

if [ -z "$2" ]; then
   linguas=$(cat "$BASEPATH/dat/LANGUAGES")
else
   linguas=$(cat "$2")
fi

for lang in $linguas; do
   lang=$(echo "$lang" | cut -c0-2)
   path="$BASEPATH/dat/gettext/${lang}/LC_MESSAGES/"
   mkdir -p "${path}"
   cp "./po/${lang}.gmo" "${path}naev.mo"
done
