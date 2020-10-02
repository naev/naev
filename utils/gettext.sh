#!/bin/bash

while read lang; do
   path="./dat/gettext/${lang}/LC_MESSAGES/"
   mkdir -p "${path}"
   cp "po/${lang}.gmo" "${path}naev.mo"
done < ./dat/LANGUAGES
