#!/bin/bash

set -e

usage() {
   echo "usage: $(basename "$0") [-v] (Verbose output)"
   cat <<EOF
Relase packaging script for Naev

This script is called by "meson install" if building for macOS with the release feature set to 'true'.

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

if ! [ -x "$(command -v zip)" ]; then
   echo "You don't have zip in PATH"
   exit 1
fi

# Clean up debug symbols bundle for debug/nightly builds
# This causes issues when signing the bundle for ARM64.

debugbundle="${MESON_INSTALL_DESTDIR_PREFIX}/Contents/MacOS/naev.dSYM"

if [ -f "$debugbundle" ] ; then
   rm "$debugbundle"
fi

# Create dist dir in build root
mkdir -p "${MESON_BUILD_ROOT}"/dist

pushd "${MESON_INSTALL_DESTDIR_PREFIX}"
cd ../
zip -r "${MESON_BUILD_ROOT}"/dist/naev-macos.zip Naev.app/*
popd
