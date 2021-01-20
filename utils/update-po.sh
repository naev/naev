#!/bin/bash

ROOT=${1-$(pwd)}

if [[ ! -f "$ROOT/naev.6" ]]; then
   echo "Please run from Naev root directory, or run with update-po.sh [source_root]"
   exit -1
fi

cd $ROOT

set -x

# For historical reasons, we generate a few sections individually. (There used to be multiple POTFILES*.in outputs.)
# Source code section:
TMPFILE=$(mktemp)
if [ -d .git ]; then
   git ls-files -- 'src/**.[ch]' ':!:*/glue_macos.h' ':!:*/khrplatform.h' ':!:*/shaders*gen*' >> "$TMPFILE"
   git ls-files -- 'dat/**.lua' >> "$TMPFILE"
else
   find src/ -name "*.[ch]" -not \( -name glue_macos.h -or -name khrplatform.h -or -name "shaders*gen*" \) >> "$TMPFILE"
   find dat/ -name "*.lua" >> "$TMPFILE"
fi

LC_ALL=C sort "$TMPFILE" > "$ROOT/po/POTFILES.in"

rm "$TMPFILE" # clean-up

# XML section
find dat/ -maxdepth 1 -name "*.xml" > "$TMPFILE"
find dat/assets -name "*.xml" >> "$TMPFILE"
find dat/outfits -name "*.xml" >> "$TMPFILE"
find dat/ships -name "*.xml" >> "$TMPFILE"
find dat/ssys -name "*.xml" >> "$TMPFILE"

LC_ALL=C sort "$TMPFILE" >> "$ROOT/po/POTFILES.in"

rm "$TMPFILE" # clean-up

# Credits section: We pull strings from the "intro" and "AUTHORS" files (inputs to credit rolls) into "credits.pot".
python "$ROOT/po/credits_pot.py" dat/intro dat/AUTHORS > "$ROOT/po/credits.pot"
echo "po/credits.pot" >> "$ROOT/po/POTFILES.in"
