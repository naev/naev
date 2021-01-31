#!/bin/bash

ROOT=${1-$(pwd)}

if [[ ! -f "$ROOT/naev.6" ]]; then
   echo "Please run from Naev root directory, or run with update-po.sh [source_root]" >&2
   exit -1
fi

cd $ROOT

# Put plaintext files before C/Lua files.
# Otherwise, xgettext's language detection "sticks" across files and *-format flags get wrongly applied to text containing '%'.

# Step 1: extract the lines of our intro/AUTHORS files (plain text).
python "$ROOT/po/credits_pot.py" dat/intro dat/AUTHORS > "$ROOT/po/credits.pot"
echo po/credits.pot > "$ROOT/po/POTFILES.in"

# Steps 2/3: list out the xml/source files in the tree.
XML_SKIP_PATTERN='/((space|ship)_polygon|unidiff)/'

if [ -d .git ]; then
   git ls-files -- 'dat/**.xml' | egrep -v "$XML_SKIP_PATTERN" | LC_ALL=C sort
   git ls-files -- 'dat/**.lua' 'src/**.[ch]' | LC_ALL=C sort
else
   find dat -name "*.xml" | egrep -v "$XML_SKIP_PATTERN" | LC_ALL=C sort
   (find dat -name "*.lua"; find src -name "*.[ch]") | LC_ALL=C sort
fi >> "$ROOT/po/POTFILES.in"
