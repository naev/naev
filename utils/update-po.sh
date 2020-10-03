#!/usr/bin/bash

ROOT=${1-$(pwd)}
TARGET=${2-"$ROOT/po/POTFILES.in"}

if [[ ! -f "$ROOT/naev.6" ]]; then
   echo "Please run from Naev root directory, or run with update-po.sh [source_root]"
   exit -1
fi

cd $ROOT

set -x

TMPFILE=$(mktemp)

echo "src/log.h" > "$TMPFILE"
find src/ -name "*.c" | sort >> "$TMPFILE"
find dat/ -name "*.lua" | sort >> "$TMPFILE"
<<<<<<< HEAD
find dat/ -name "*.xml" -maxdepth 1 | sort >> "$TMPFILE"
=======
find dat/ -maxdepth 1 -name "*.xml" | sort >> "$TMPFILE"
>>>>>>> master
find dat/assets -name "*.xml" | sort >> "$TMPFILE"
find dat/outfits -name "*.xml" | sort >> "$TMPFILE"
find dat/ships -name "*.xml" | sort >> "$TMPFILE"
find dat/ssys -name "*.xml" | sort >> "$TMPFILE"

# Remove file if found
sed -i '/src\/shaders.gen.c/d' "$TMPFILE"

cat "$TMPFILE" | sort > $TARGET

