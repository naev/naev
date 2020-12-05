#!/bin/bash

set -ex

if [[ -z "$1" ]]; then
   BASEPATH="."
else
   BASEPATH="$1"
fi

if [[ -z "$2" ]]; then
   linguas=$(<"$BASEPATH/dat/LANGUAGES")
else
   linguas=$(<"$2")
fi

for lang in $linguas; do
   lang=${lang:0:2}
   path="$BASEPATH/dat/gettext/${lang}/LC_MESSAGES/"
   mkdir -p "${path}"
   cp "./po/${lang}.gmo" "${path}naev.mo"
done
