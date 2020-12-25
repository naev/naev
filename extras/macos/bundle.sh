#!/bin/bash

# After building Naev, run this script from the toplevel source directory to
# create Naev.app.
# FIXME: The real instructions are more like: cd to Meson build directory, fix bugs in script, use script.

# This script assumes the environment we set up in Travis, and copies the
# dependencies to the bundle. These are:
#
# - From Homebrew: freetype, libpng, libvorbis, luajit, sdl2, sdl2_image

set -e

# Clean previous build.
rm -fr Naev.app

# Build basic structure.
mkdir -p Naev.app/Contents/{MacOS,Resources,Frameworks}/
cp extras/macos/Info.plist Naev.app/Contents/
cp extras/macos/naev.icns Naev.app/Contents/Resources/
cp src/naev Naev.app/Contents/MacOS/

# Find all Homebrew dependencies (from /usr/local),
# and descend to find deps of deps.
prev_deps=""
# Gather Naev direct dependencies.
deps="$(
  otool -L src/naev |
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
install -m 0644 $deps Naev.app/Contents/Frameworks

# We need to fix the dylib paths, changing them to use @rpath.
# Collect -change args here for install_name_tool.
change_args=""
for dep in $deps; do
  change_args+=" -change $dep @rpath/$(basename $dep)"
done

# Apply changes to the Naev executable, and add the rpath entry.
install_name_tool $change_args \
  -add_rpath @executable_path/../Frameworks \
  Naev.app/Contents/MacOS/naev

# Apply changes to bundled dylibs too, and set their dylib ID.
for dep in Naev.app/Contents/Frameworks/*.dylib; do
  install_name_tool $change_args \
    -id @rpath/$(basename $dep) \
    $dep
done

# Strip headers, especially from the SDL2 framework.
find Naev.app -name Headers -prune | xargs rm -r

echo "Successfully created Naev.app"
