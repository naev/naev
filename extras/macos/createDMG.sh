#!/usr/bin/env bash

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

if ! [ -x "$(command -v mkfs.hfsplus)" ]; then
   echo "You don't have mkfs.hfsplus in PATH"
   echo "Install hfsprogs (Debian/Ubuntu) or ensure the tool is available."
   exit 1
elif ! [ -x "$(command -v hfsplus)" ]; then
   echo "You don't have the hfsplus tool in PATH"
   echo "Install libdmg-hfsplus (provides hfsplus)."
   exit 1
elif ! [ -x "$(command -v dmg)" ]; then
   echo "You don't have dmg in PATH"
   echo "Get it from https://github.com/fanquake/libdmg-hfsplus"
   exit 1
fi


# Creates temp directory
WORKPATH="${MESON_BUILD_ROOT}/dmg_staging"
dmg_out="${MESON_BUILD_ROOT}/dist/naev-macos.dmg"
hfs_image="${MESON_BUILD_ROOT}/naev-macos.hfsplus"

# Create dist dir in build root and a clean staging area
mkdir -p "${MESON_BUILD_ROOT}/dist"
rm -rf "$WORKPATH"
mkdir -p "$WORKPATH"

# Copy all DMG assets to staging
cp -r "${MESON_SOURCE_ROOT}"/extras/macos/dmg_assets/. "$WORKPATH"

# Extract Naev app bundle to staging
cp -r "${MESON_INSTALL_DESTDIR_PREFIX}" "$WORKPATH"

# Size an HFS+ image with padding and build it
size_kb=$(du -sk "$WORKPATH" | cut -f1)
padding_kb=51200 # ~50MB padding for filesystem metadata and safety margin
image_kb=$((size_kb + padding_kb))
rm -f "$hfs_image"
truncate -s "${image_kb}K" "$hfs_image"
mkfs.hfsplus -v "Naev" "$hfs_image"

# Populate the HFS+ image and convert it to a compressed DMG
hfsplus "$hfs_image" addall "$WORKPATH" /
# Add the Applications symlink inside the HFS+ image (avoid resolving it on the host)
hfsplus "$hfs_image" rm /Applications >/dev/null 2>&1 || true
hfsplus "$hfs_image" symlink /Applications /Applications
rm -f "$dmg_out"
dmg -c lzma -l 8 build "$hfs_image" "$dmg_out"

# Clean up after ourselves (Your storage space will thank me.)
rm -rf "$WORKPATH" "$hfs_image"
