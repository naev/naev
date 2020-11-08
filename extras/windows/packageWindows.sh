#!/bin/bash
# WINDOWS PACKAGING SCRIPT FOR NAEV
# Requires NSIS to be installed
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

if [[ $PATH == *"mingw64"* ]]; then
    echo "Detected MinGW64 environment"
    ARCH="64"
else
    echo "Make sure you are running this in an MSYS2 64bit MinGW environment"
    exit -1
fi

echo "ARCH:      $ARCH"

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
mkdir -p "$SOURCEROOT/extras/windows/installer/bin"

# Move data to staging folder
echo "moving data to staging area"
cp -r "$SOURCEROOT/dat" "$SOURCEROOT/extras/windows/installer/bin"

# Collect DLLs
 
if [[ $ARCH == "64" ]]; then
for fn in `cygcheck "$BUILDPATH/naev.exe" | grep "mingw64"`; do
    echo "copying $fn to staging area"
    cp "$fn" "$SOURCEROOT/extras/windows/installer/bin"
done
else
    echo "Aw, man, I shot Marvin in the face..."
    echo "Something went wrong while looking for DLLs to stage."
    exit -1
fi

echo "copying naev logo to staging area"
cp "$SOURCEROOT/extras/logos/logo.ico" "$SOURCEROOT/extras/windows/installer"

echo "copying naev binary to staging area"
cp "$BUILDPATH/naev.exe" "$SOURCEROOT/extras/windows/installer/bin/naev-$SUFFIX.exe"

# Create distribution folder

echo "creating distribution folder if it doesn't exist"
mkdir -p "$BUILDOUTPUT/out"

# Build installer

makensis -DSUFFIX=$SUFFIX "$SOURCEROOT/extras/windows/installer/naev.nsi"

# Move installer to distribution directory
mv "$SOURCEROOT/extras/windows/installer/naev-$SUFFIX.exe $BUILDOUTPUT/out"

echo "Successfully built Windows Installer for $SUFFIX"

# Package steam windows tarball
pushd "$SOURCEROOT/extras/windows/installer/bin"
tar -cJvf ../steam-win64.tar.xz *.dll *.exe
mv ../*.xz "$BUILDOUTPUT/out"
popd

echo "Successfully packaged Steam Tarball"

echo "Cleaning up staging area"
rm -rf "$SOURCEROOT/extras/windows/installer/bin"
rm -rf "$SOURCEROOT/extras/windows/installer/logo.ico"
