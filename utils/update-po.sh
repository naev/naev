#!/usr/bin/bash

if [[ ! -f "naev.6" ]]; then
   echo "Please run from Naev root directory."
   exit -1
fi

set -x

# General file
TMPFILE=$(mktemp)
echo "src/log.h" > "$TMPFILE"
find src/ -name "*.c" >> "$TMPFILE"
find dat/ -name "*.lua" >> "$TMPFILE"
echo "po/xml.pot" >> "$TMPFILE"
# Remove file if found
sed -i '/src\/shaders.gen.c/d' "$TMPFILE"
cat "$TMPFILE" | sort > po/POTFILES.in
rm "$TMPFILE" # clean-up

# XML file
find dat/ -maxdepth 1 -name "*.xml" > "$TMPFILE"
find dat/assets -name "*.xml" >> "$TMPFILE"
find dat/outfits -name "*.xml" >> "$TMPFILE"
find dat/ships -name "*.xml" >> "$TMPFILE"
cat "$TMPFILE" | sort > po/POTFILES_XML
rm "$TMPFILE" # clean-up
