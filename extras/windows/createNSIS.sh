#!/bin/bash

set -e

usage() {
   echo "usage: $(basename "$0") [-v] (Verbose output)"
   cat <<EOF
NSIS installer compilation script for Naev
This script is called by "meson install" when building on Windows, and with the 'installer' option selected.

It builds an NSIS installer for Naev

Requires "makensis" to be installed and available on PATH.

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

if ! [ -x "$(command -v makensis)" ]; then
   echo "You don't have makensis in PATH"
   exit 1
fi

# build temp directory from meson stuff
WORKPATH="${MESON_BUILD_ROOT}/installer_staging"

# Make temp directory
mkdir -p "$WORKPATH/bin"

# Create dist folder
mkdir -p "${MESON_BUILD_ROOT}/dist"

# Copy all installer assets to installer staging area.
cp -r "${MESON_SOURCE_ROOT}"/extras/windows/installer_assets/. "${MESON_SOURCE_ROOT}"/LICENSE "${MESON_INSTALL_DESTDIR_PREFIX}"/*.ico "$WORKPATH"
mv "$WORKPATH"/LICENSE "$WORKPATH"/legal/naev-license.txt

# copy Windows DLLs, binary and dat files to installer staging area.
cp -r "${MESON_INSTALL_DESTDIR_PREFIX}/." "$WORKPATH/bin"

# Set VERSION.
VERSION="$(<"${MESON_INSTALL_DESTDIR_PREFIX}/dat/VERSION")"

# Compile NSIS installer
makensis "-XOutFile ${MESON_BUILD_ROOT}/dist/naev-installer.exe" -DVERSION="$VERSION" "$WORKPATH/naev.nsi"

#Clean up
rm -rf "$WORKPATH"
