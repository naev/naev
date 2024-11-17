#!/bin/bash

set -e

usage() {
   echo "usage: $(basename "$0") [-v] (Verbose output)"
   cat <<EOF
DMG Packaging Script for Naev

This script is called by "meson install" if building for macOS.

usage: $(basename "$0") [-v] (Verbose output)
EOF
   exit 1
}

while getopts v OPTION "$@"; do
   case $OPTION in
      v)
         set -x
         ;;

      *)
         usage
         ;;
   esac
done

if ! [ -x "$(command -v genisoimage)" ]; then
   echo "You don't have genisoimage in PATH"
   exit 1
elif ! [ -x "$(command -v dmg)" ]; then
   echo "You don't have dmg in PATH"
   echo "Get it from https://github.com/fanquake/libdmg-hfsplus"
   exit 1
elif ! [ -x "$(command -v readlink)" ]; then
   echo "You don't have readlink in PATH"
   echo "Get it from your distro repositories."
   exit 1
fi


# Creates temp directory
WORKPATH="${MESON_BUILD_ROOT}/dmg_staging"


# Create dist dir in build root
mkdir -p "${MESON_BUILD_ROOT}"/dist

# Create temp directory in build root
mkdir -p "$WORKPATH"

# Create Applications symlink for DMG installer in temp directory
ln -s /Applications "$WORKPATH"/Applications

# Copy all DMG assets to BundleDir
cp -r "${MESON_SOURCE_ROOT}"/extras/macos/dmg_assets/. "$WORKPATH"

# Extract Naev app bundle to BundleDir
cp -r "${MESON_INSTALL_DESTDIR_PREFIX}" "$WORKPATH"

# Generate ISO image and compress into DMG
genisoimage -V Naev -D -R -apple -no-pad -o "$WORKPATH"/naev-macos.iso "$WORKPATH"
dmg "$WORKPATH"/naev-macos.iso "${MESON_BUILD_ROOT}"/dist/naev-macos.dmg

# Clean up after ourselves (Your storage space will thank me.)
rm -rf "$WORKPATH"
