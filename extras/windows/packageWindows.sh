#!/bin/bash
# WINDOWS PACKAGING SCRIPT FOR NAEV
# Requires NSIS and python3-pip to be installed
#
# This script should be run after compiling Naev
# It detects the current environment, and builds the appropriate NSIS installer
# into the root naev directory.
#
# Pass in [-d] [-n] (set this for nightly builds) -s <SOURCEROOT> (Sets location of source) -b <BUILDROOT> (Sets location of build directory) -o <BUILDOUTPUT> (Dist output directory)

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

BUILD_DATE="$(date +%Y%m%d)"

# Output configured variables

echo "SOURCE ROOT:  $SOURCEROOT"
echo "BUILD ROOT:   $BUILDPATH"
echo "NIGHTLY:      $NIGHTLY"
echo "BUILD OUTPUT: $BUILDOUTPUT"

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

echo "creating staging area"
STAGING="$SOURCEROOT/extras/windows/installer/bin"
mkdir -p "$STAGING"

# Move data to staging folder
echo "moving data to staging area"
cp -r "$SOURCEROOT/dat" "$STAGING"

# Collect DLLs
echo "Populating staging area with Meson subprojects' DLLs (if any)"
find "${SOURCEROOT}/subprojects" -iname "*.dll" -exec cp -v "{}" "$STAGING" ";"
echo "Locally install 'pefile' Python module"
python3 -m pip install pefile
echo "Collecting DLLs in staging area"
"$SOURCEROOT"/extras/windows/extract_dlls.py "$BUILDPATH/naev.exe" "$STAGING"

echo "copying naev logo to staging area"
cp "$SOURCEROOT/extras/logos/logo.ico" "$SOURCEROOT/extras/windows/installer"

echo "copying naev binary to staging area"
cp "$BUILDPATH/naev.exe" "$STAGING/naev-$SUFFIX.exe"

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
mv ../*.xz "$BUILDOUTPUT/out"
popd

echo "Successfully packaged Steam Tarball"

echo "Cleaning up staging area"
rm -rf "$STAGING"
rm -rf "$SOURCEROOT/extras/windows/installer/logo.ico"
