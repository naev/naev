#!/bin/bash

# WINDOWS INSTALL SCRIPT FOR NAEV
# This script is called by "meson install" if building for Windows.
# It builds the appropriate NSIS installer into the root naev directory.
# Requires mingw-w64-x86_64-nsis to be installed, and the submodule in extras/windows/mingw-bundledlls to be available.

set -e

# Defaults
SOURCEROOT="${MESON_SOURCE_ROOT}"
BUILDPATH="${MESON_BUILD_ROOT}"
NIGHTLY="false"
BUILDOUTPUT="$(pwd)/dist"

while getopts n: OPTION "$@"; do
    case $OPTION in
    n)
        NIGHTLY="${OPTARG}"
        ;;
    esac
done

BUILD_DATE="$(date +%Y%m%d)"

# Output configured variables

echo "SOURCE ROOT:  $SOURCEROOT"
echo "BUILD ROOT:   $BUILDPATH"
echo "NIGHTLY:      $NIGHTLY"
echo "BUILD OUTPUT: $BUILDOUTPUT"

# MinGW DLL search paths
MINGW_BUNDLEDLLS_SEARCH_PATH="/mingw64/bin:/usr/x86_64-w64-mingw32/bin:/usr/lib/mxe/usr/x86_64-w64-mingw32.shared/bin"
# Include all subdirs (mingw-bundledlls can't search recursively) of in-tree and out-of-tree subproject dirs.
# Normally, Meson builds everything out-of-tree, but some subprojects have their own build systems which do as they please.
for MESON_SUBPROJ_DIR in "${SOURCEROOT}/subprojects" "${BUILDPATH}/subprojects"; do
    echo "Searching ${MESON_SUBPROJ_DIR}"
    if [ -d "${MESON_SUBPROJ_DIR}" ]; then
        MINGW_BUNDLEDLLS_SEARCH_PATH+=$(find "${MESON_SUBPROJ_DIR}" -type d -printf ":%p")
    fi
done
export MINGW_BUNDLEDLLS_SEARCH_PATH

# Rudementary way of detecting which environment we are packaging.. 
# It works (tm), and it should remain working until msys changes their naming scheme

# Check version exists and set VERSION variable.

if [ -f "$SOURCEROOT/dat/VERSION" ]; then
    VERSION="$(<"$SOURCEROOT/dat/VERSION")"
else
    echo "The VERSION file is missing from $SOURCEROOT."
    exit -1
fi
if [[ "$NIGHTLY" == "true" ]]; then
    export VERSION="$VERSION.$BUILD_DATE"
fi
SUFFIX="$VERSION-win64"

# Move compiled binary to staging folder.

STAGING="$SOURCEROOT/extras/windows/installer/bin"

# Move data to staging folder
echo "copying naev logo to staging area"
cp "$SOURCEROOT/extras/logos/logo.ico" "$SOURCEROOT/extras/windows/installer"

echo "copying naev binary to staging area"
mv "$STAGING/naev.exe" "$STAGING/naev-$SUFFIX.exe"

# Collect DLLs
echo "Collecting DLLs in staging area"
python3 "$SOURCEROOT/extras/windows/mingw-bundledlls/mingw-bundledlls" --copy "$STAGING/naev-$SUFFIX.exe"

# Create distribution folder

echo "creating distribution folder if it doesn't exist"
mkdir -p "$BUILDOUTPUT/out"

# Build installer

makensis -DSUFFIX=$SUFFIX "$SOURCEROOT/extras/windows/installer/naev.nsi"

# Move installer to distribution directory
mv "$SOURCEROOT/extras/windows/installer/naev-$SUFFIX.exe" "$BUILDOUTPUT/out"

echo "Successfully built Windows Installer for $SUFFIX"

# Package steam windows tarball
pushd "$STAGING"
tar -cJvf ../steam-win64.tar.xz *.dll *.exe
popd
mv "$STAGING"/../*.xz "$BUILDOUTPUT/out"

echo "Successfully packaged Steam Tarball"

echo "Cleaning up staging area"
rm -rf "$STAGING"
rm -rf "$SOURCEROOT/extras/windows/installer/logo.ico"
