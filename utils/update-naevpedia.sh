#!/usr/bin/env bash

set -e

# ============================= Enter source root =============================
if [ -n "$1" ]; then
   cd "$1"
elif [ -n "$MESON_SOURCE_ROOT" ]; then
   cd "$MESON_SOURCE_ROOT"
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
   find_files() { find "$1" -name "*.$2"; }
fi
# And some pipeline commands:
deterministic_sort() { LC_ALL=C sort; }

# =================================== Main ===================================

SHIPSFILE="dat/naevpedia/ships/meson.build"
mkdir -p "$(dirname "${SHIPSFILE}")"
echo 'root = meson.project_source_root()' > ${SHIPSFILE}
echo 'naevpedia_ships_sources = files(' >> ${SHIPSFILE}
find_files dat/ships xml | deterministic_sort | xargs -I {} sh -c "echo \"   root / '{}',\" >> ${SHIPSFILE}"
echo ')' >> ${SHIPSFILE}
echo "
ships_gen = [find_program('ships.py'), '@INPUT@', '-o', '@OUTPUT@']
naevpedia_ships = []
foreach ns: naevpedia_ships_sources
  naevpedia_ships += custom_target( ns.full_path().replace('/','_'),
      command: ships_gen,
      input: ns,
      output: '@BASENAME@.md',
      install: true,
      install_dir: ndata_path / 'dat/naevpedia/ships',
    )
endforeach" >> ${SHIPSFILE}

OUTFITSFILE="dat/naevpedia/outfits/meson.build"
mkdir -p "$(dirname "${OUTFITSFILE}")"
echo 'root = meson.project_source_root()' > ${OUTFITSFILE}
echo 'naevpedia_outfits_sources = files(' >> ${OUTFITSFILE}
find_files dat/outfits xml | deterministic_sort | xargs -I {} sh -c "echo \"   root / '{}',\" >> ${OUTFITSFILE}"
echo ')' >> ${OUTFITSFILE}
echo "
outfits_gen = [find_program('outfits.py'), '@INPUT@', '-o', '@OUTPUT@']
naevpedia_outfits = []
foreach ns: naevpedia_outfits_sources
  naevpedia_outfits += custom_target( ns.full_path().replace('/','_'),
      command: outfits_gen,
      input: ns,
      output: '@BASENAME@.md',
      install: true,
      install_dir: ndata_path / 'dat/naevpedia/outfits',
    )
endforeach

genlist = []
foreach gen: gen_outfits
  genlist += gen.to_list()
endforeach
foreach ns: genlist
  naevpedia_outfits += custom_target( ns.full_path().replace('/','_'),
      command: outfits_gen,
      depends: gen_outfits, # a bit brute force to depend on gen_outfit stage, but works
      input: ns,
      output: '@BASENAME@.md',
      install: true,
      install_dir: ndata_path / 'dat/naevpedia/outfits',
    )
endforeach" >> ${OUTFITSFILE}

if [ "$2" = "--pre-commit" ]; then
   # The "pre-commit" package requires hooks to fail if they touch any files.
   git diff --exit-code "${SHIPSFILE}" && exit 0
   git diff --exit-code "${OUTFITSFILE}" && exit 0
   echo Updating naevpedia!
fi
