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

# Create dist dir in build root
mkdir -p "${MESON_BUILD_ROOT}"/dist

pushd "${MESON_INSTALL_DESTDIR_PREFIX}"
cd ../
zip -r "${MESON_BUILD_ROOT}"/dist/naev-macos.zip Naev.app/*
popd
