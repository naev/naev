#!/bin/bash

if [[ -z "$1" ]]; then
   BASEPATH="."
else
   BASEPATH="$1"
fi

linguas=`cat $BASEPATH/dat/LANGUAGES`
for lang in $linguas; do
   lang=${lang:0:2}
   path="$BASEPATH/dat/gettext/${lang}/LC_MESSAGES/"
   mkdir -p "${path}"
   cp "./po/${lang}.gmo" "${path}naev.mo"
done
