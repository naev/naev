#!/bin/bash

set -e

usage() {
   echo "usage: $(basename "$0") [-v] (Verbose output)"
   cat <<EOF
Relase packaging script for Naev

This script is called by "meson install" if building for Windows with the release feature set to 'true'.

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

if ! [ -x "$(command -v tar)" ]; then
   echo "You don't have tar in PATH"
   exit 1
fi

# Create dist folder
mkdir -p "${MESON_BUILD_ROOT}/dist"

# Package steam windows tarball
pushd "${MESON_INSTALL_DESTDIR_PREFIX}"
tar -cJvf naev-windows.tar.xz -- *.dll *.exe
popd
mv "${MESON_INSTALL_DESTDIR_PREFIX}/naev-windows.tar.xz" "${MESON_BUILD_ROOT}/dist"
