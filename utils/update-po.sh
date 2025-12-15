#!/bin/bash

set -e

# ============================= Enter source root =============================
if [ -n "$1" ]; then
   cd "$1"
elif [ -n "$MESON_SOURCE_ROOT" ]; then
   cd "$MESON_SOURCE_ROOT"
fi
if [ -n "$2" ]; then
   BUILDDIR="$2"
else
   BUILDDIR="."
fi

if [[ ! -f naev.6 ]]; then
   echo "Please run from the source root dir, or pass it as an argument." >&2
   exit 1
fi

# ============================== Helper commands ==============================
# find_files <dir> <suffix>
if [ -d .git ]; then
   unset GIT_INDEX_FILE  # Don't let whatever pre-commit does fuck up the results.
   find_files() { git ls-files -- "$1/**.$2"; }
else
   find_files() {
      find dat -name "*.xml"
      (find dat -name "*.lua"; find src -name "*.[ch]")
   }
fi
# And some pipeline commands:
filter_skipped() { grep -vE '/((space|ship)_polygon)/.*\.xml'; }
deterministic_sort() { LC_ALL=C sort; }

# =================================== Main ===================================

# Prepare POTFILES.in (which lists all files with translatable text).
# We also have plaintext files, whose lines should all be translatable strings.
# We collect these strings into a .pot file, and let xgettext handle the rest.
# The text strings must come first, or else gettext "remembers" its most recent
# language detection and gives them unwanted "c-format" or "lua-format" tags.

IFS=$'\n'
readarray -t ART <<< "$(cd assets; find_files gfx/loading txt | sed 's|^|assets/|')"
po/credits_pot.py po/credits.pot dat/AUTHORS "${ART[@]}"
po/toml_pot.py po/toml.pot dat/damagetype/ dat/slots/ dat/start.toml

# Only update naevpedia if not run from pre-commit.
# This is because naevpedia generates files and we can only update them with
# the build directory known, aka from inside meson.
if [ "$3" != "--pre-commit" ]; then
   readarray -t MD1 <<< "$(cd dat; find_files naevpedia md | sed 's|^|dat/|')"
   readarray -t MD2 <<< "$(find "${BUILDDIR}/dat/naevpedia" -name "*.md")"
   po/naevpedia_pot.py po/naevpedia.pot "${MD1[@]}" "${MD2[@]}"

   readarray -t MD3 <<< "$(find "${BUILDDIR}/dat/outfits" -name "*.xml")"
   xgettext --its "$1/po/its/translation.its" "${MD3[@]}" -o "po/outfits_generated.pot"
   sed -i 's/CHARSET/UTF-8/' "po/outfits_generated.pot"
fi

# Generate the POTFILES.in using the sub files and the rest of the stuff
TMPFILE=$(mktemp)
(
   echo po/naevpedia.pot
   echo po/physfs.pot
   echo po/credits.pot
   echo po/toml.pot
   echo po/outfits_generated.pot
   find_files dat xml | deterministic_sort
   ( find_files dat lua; find_files src "[ch]"; find_files src rs) | deterministic_sort
   find_files dat/outfits py | deterministic_sort
) | filter_skipped > "$TMPFILE"

cmp --silent "$TMPFILE" po/POTFILES.in || cp "$TMPFILE" po/POTFILES.in
rm "$TMPFILE"

# Finally, the "pre-commit" package requires hooks to fail if they touch any files.
if [ "$3" = "--pre-commit" ]; then
   git diff --exit-code po/POTFILES.in && exit 0
   echo "Fixing po/POTFILES.in"
else
   cp po/POTFILES.in "$BUILDDIR/$3"
fi
