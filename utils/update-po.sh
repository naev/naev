#!/usr/bin/bash

ROOT=${1-$(pwd)}

if [[ ! -f "$ROOT/naev.6" ]]; then
   echo "Please run from Naev root directory, or run with update-po.sh [source_root]"
   exit -1
fi

cd $ROOT

set -x

# General file
TMPFILE=$(mktemp)
echo "po/credits.pot" >> "$TMPFILE"
find src/ -name "*.[ch]" -not \( -name "glue_macos.h" -or -name "khrplatform.h" -or -name "shaders*gen*" \) >> "$TMPFILE"
find dat/ -name "*.lua" >> "$TMPFILE"

cat "$TMPFILE" | LC_ALL=C sort | tee "$ROOT/po/POTFILES_COMBINED.in" > "$ROOT/po/POTFILES.in"

rm "$TMPFILE" # clean-up

# XML file
find dat/ -maxdepth 1 -name "*.xml" > "$TMPFILE"
find dat/assets -name "*.xml" >> "$TMPFILE"
find dat/outfits -name "*.xml" >> "$TMPFILE"
find dat/ships -name "*.xml" >> "$TMPFILE"
find dat/ssys -name "*.xml" >> "$TMPFILE"

cat "$TMPFILE" | LC_ALL=C sort | tee -a "$ROOT/po/POTFILES_COMBINED.in" > "$ROOT/po/POTFILES_XML.in"
echo "po/xml.pot" >> "$ROOT/po/POTFILES.in"

rm "$TMPFILE" # clean-up

# Pull strings from the "intro" and "AUTHORS" files (inputs to credit rolls) into "credits.pot".
awk -f "$ROOT/utils/update-po-credits.awk" dat/intro dat/AUTHORS > "$ROOT/po/credits.pot"
