#!/bin/bash

# MACOS PACKAGING SCRIPT FOR NAEV
# This script should be run after compiling Naev.
#
# Pass in [-d] [-n] (set this for nightly builds) -s <SOURCEROOT> (Sets location of source) -b <BUILDROOT> (Sets location of build directory) -o <BUILDOUTPUT> (Dist output directory)

# This script assumes the environment we set up in Travis, and copies the
# dependencies to the bundle. These are:
#
# - From Homebrew: freetype, libpng, libvorbis, luajit, sdl2, sdl2_image

set -e

# Defaults
SOURCEROOT="$(pwd)"
BUILDPATH="$(pwd)/build"
NIGHTLY="false"
BUILDOUTPUT="$(pwd)/dist"

while getopts dns:b:o: OPTION "$@"; do
    case $OPTION in
    d)
        set -x
        ;;
    n)
        NIGHTLY="true"
        ;;
    s)
        SOURCEROOT="${OPTARG}"
        ;;
    b)
        BUILDPATH="${OPTARG}"
        ;;
    o)
        BUILDOUTPUT="${OPTARG}"
        ;;
        
    esac
done

# Clean previous build.
rm -fr "${BUILDOUTPUT}"/Naev.app

# Build basic structure.
mkdir -p "${BUILDOUTPUT}"/Naev.app/Contents/{MacOS,Resources,Frameworks}/
cp "${SOURCEROOT}"/extras/macos/Info.plist "${BUILDOUTPUT}"/Naev.app/Contents/
cp "${SOURCEROOT}"/extras/macos/naev.icns "${BUILDOUTPUT}"/Naev.app/Contents/Resources/
cp "${BUILDPATH}"/naev "${BUILDOUTPUT}"/Naev.app/Contents/MacOS/

# Find all Homebrew dependencies (from /usr/local),
# and descend to find deps of deps.
prev_deps=""
# Gather Naev direct dependencies.
deps="$(
  otool -L "${BUILDPATH}"/naev |
    perl -ne '/(\/usr\/local\/.+.dylib)/ && print "$1\n"'
)"
# Iterate while the deps array keeps changing.
while [[ "${deps}" != "${prev_deps}" ]]; do
  prev_deps="${deps}"
  # For each dep, find deps of that dep.
  for dep in $deps; do
    depdeps="$(
      otool -L $dep |
        perl -ne '/(\/usr\/local\/.+.dylib)/ && print "$1\n"'
    )"
    for depdep in $depdeps; do
      # Check if the dep was already picked up, or add it.
      if [[ $deps != *"${depdep}"* ]]; then
        deps+=" ${depdep}"
      fi
    done
  done
done
# Log the list.
echo "Bundling Homebrew dependencies:"
echo $deps | tr " " "\n"

# Bundle the Homebrew dependencies.
install -m 0644 $deps "${BUILDOUTPUT}"/Naev.app/Contents/Frameworks

# We need to fix the dylib paths, changing them to use @rpath.
# Collect -change args here for install_name_tool.
change_args=""
for dep in $deps; do
  change_args+=" -change $dep @rpath/$(basename $dep)"
done

# Apply changes to the Naev executable, and add the rpath entry.
install_name_tool $change_args \
  -add_rpath @executable_path/../Frameworks \
  "${BUILDOUTPUT}"/Naev.app/Contents/MacOS/naev

# Apply changes to bundled dylibs too, and set their dylib ID.
for dep in "${BUILDOUTPUT}"/Naev.app/Contents/Frameworks/*.dylib; do
  install_name_tool $change_args \
    -id @rpath/$(basename $dep) \
    $dep
done

# Strip headers, especially from the SDL2 framework.
find "${BUILDOUTPUT}"/Naev.app -name Headers -prune | xargs rm -r

echo "Successfully created "${BUILDOUTPUT}"/Naev.app"
