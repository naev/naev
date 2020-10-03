#!/bin/bash

linguas=`cat ./dat/LANGUAGES`
for lang in $linguas; do
   lang=${lang:0:2}
   path="./dat/gettext/${lang}/LC_MESSAGES/"
   mkdir -p "${path}"
   cp "po/${lang}.gmo" "${path}naev.mo"
done
