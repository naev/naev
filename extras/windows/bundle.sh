#!/bin/bash

set -e

usage() {
   echo "usage: $(basename "$0") [-v] (Verbose output)"
   cat <<EOF
DLL Bundler for Windows

This script is called by "meson install" if building for Windows.
The intention is for this wrapper to behave in a similar manner to the bundle.py script in extras/macos.

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

# MinGW DLL search paths
MINGW_BUNDLEDLLS_SEARCH_PATH="/mingw64/bin:/usr/x86_64-w64-mingw32/bin:/usr/x86_64-w64-mingw32/sys-root/mingw/bin:/usr/lib/mxe/usr/x86_64-w64-mingw32.shared/bin"
# Include all subdirs (mingw-bundledlls can't search recursively) of in-tree and out-of-tree subproject dirs.
# Normally, Meson builds everything out-of-tree, but some subprojects have their own build systems which do as they please.
for MESON_SUBPROJ_DIR in "${MESON_SOURCE_ROOT}/subprojects" "${MESON_BUILD_ROOT}/subprojects"; do
    echo "Searching ${MESON_SUBPROJ_DIR}"
    if [ -d "${MESON_SUBPROJ_DIR}" ]; then
        MINGW_BUNDLEDLLS_SEARCH_PATH+=$(find "${MESON_SUBPROJ_DIR}" -type d -printf ":%p")
    fi
done
export MINGW_BUNDLEDLLS_SEARCH_PATH

DLL_LIST=$("${MESON_SOURCE_ROOT}/extras/windows/mingw-bundledlls/mingw-bundledlls" "${MESON_BUILD_ROOT}/naev.exe")

for DLL in $DLL_LIST; do
cp "$DLL" "${MESON_INSTALL_DESTDIR_PREFIX}"
done
