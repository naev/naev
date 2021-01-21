#!/bin/bash

ROOT=${1-$(pwd)}

if [[ ! -f "$ROOT/naev.6" ]]; then
   echo "Please run from Naev root directory, or run with update-po.sh [source_root]"
   exit -1
fi

cd $ROOT

# For historical reasons, we have a strange sort order.
# Source code section:
if [ -d .git ]; then
   git ls-files -- 'src/**.[ch]' 'dat/**.lua' | LC_ALL=C sort > "$ROOT/po/POTFILES.in"
else
   (find src/ -name "*.[ch]"; find dat/ -name "*.lua") | LC_ALL=C sort > "$ROOT/po/POTFILES.in"
fi

# XML section
find dat/ -name "*.xml" | egrep -v '/((space|ship)_polygon|unidiff)/' | LC_ALL=C sort >> "$ROOT/po/POTFILES.in"

# Credits section: We pull strings from the "intro" and "AUTHORS" files (inputs to credit rolls) into "credits.pot".
python "$ROOT/po/credits_pot.py" dat/intro dat/AUTHORS > "$ROOT/po/credits.pot"
echo "po/credits.pot" >> "$ROOT/po/POTFILES.in"
