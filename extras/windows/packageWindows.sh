#!/bin/bash
# WINDOWS PACKAGING SCRIPT FOR NAEV
# Requires NSIS to be installed
#
# This script should be run after compiling Naev
# It detects the current environment, and builds the appropriate NSIS installer
# into the root naev directory.
#
# Pass in [-d] [-n] (set this for nightly builds) -s <SOURCEROOT> (Sets location of source) -o <OUTPUTPATH> (dist output directory)

set -e

# Defaults
SOURCEROOT="$(pwd)"
BUILDDIR="$(pwd)/build"
NIGHTLY="false"
OUTPUTPATH="$(pwd)/dist"

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
        BUILDDIR="${OPTARG}"
        ;;
    o)
        OUTPUTPATH="${OPTARG}"
        ;;
        
    esac
done

BUILD_DATE="$(date +%Y%m%d)"

# Output configured variables

echo "SOURCE ROOT:  $SOURCEROOT"
echo "BUILD ROOT:   $BUILDDIR"
echo "NIGHTLY:      $NIGHTLY"
echo "BUILD OUTPUT: $OUTPUTPATH"

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

if test -f "$SOURCEROOT/dat/VERSION"; then
    VERSION="$(cat $SOURCEROOT/dat/VERSION)"
else
    echo "The VERSION file is missing from $SOURCEROOT."
    exit -1
fi

# Move compiled binary to staging folder.

echo "creating staging area"
mkdir -p $SOURCEROOT/extras/windows/installer/bin

# Move data to staging folder
echo "moving data to staging area"
cp -r $SOURCEROOT/dat $SOURCEROOT/extras/windows/installer/bin

# Collect DLLs
 
if [[ $ARCH == "64" ]]; then
for fn in `cygcheck "$BUILDDIR/naev.exe" | grep "mingw64"`; do
    echo "copying $fn to staging area"
    cp $fn $SOURCEROOT/extras/windows/installer/bin
done
else
    echo "Aw, man, I shot Marvin in the face..."
    echo "Something went wrong while looking for DLLs to stage."
    exit -1
fi

echo "copying naev logo to staging area"
cp $SOURCEROOT/extras/logos/logo.ico $SOURCEROOT/extras/windows/installer

echo "copying naev binary to staging area"
if [[ $NIGHTLY == true ]]; then
cp $BUILDDIR/naev.exe $SOURCEROOT/extras/windows/installer/bin/naev-$VERSION.$BUILD_DATE-win$ARCH.exe
elif [[ $NIGHTLY == false ]]; then
cp $BUILDDIR/naev.exe $SOURCEROOT/extras/windows/installer/bin/naev-$VERSION-win$ARCH.exe
else
    echo "Cannot think of another movie quote."
    echo "Something went wrong while copying binary to staging area."
    exit -1
fi

# Create distribution folder

echo "creating distribution folder if it doesn't exist"
mkdir -p $OUTPUTPATH/out

# Build installer

if [[ $NIGHTLY = true ]]; then
    makensis -DVERSION=$VERSION.$BUILD_DATE -DARCH=$ARCH $SOURCEROOT/extras/windows/installer/naev.nsi

    # Move installer to distribution directory
    mv $SOURCEROOT/extras/windows/installer/naev-$VERSION.$BUILD_DATE-win$ARCH.exe $OUTPUTPATH/out

elif [[ $NIGHTLY == false ]]; then
    makensis -DVERSION=$VERSION -DARCH=$ARCH $SOURCEROOT/extras/windows/installer/naev.nsi

    # Move installer to distribution directory
    mv $SOURCEROOT/extras/windows/installer/naev-$VERSION-win$ARCH.exe $OUTPUTPATH/out
else
    echo "Cannot think of another movie quote.. again."
    echo "Something went wrong.."
    exit -1
fi

echo "Successfully built Windows Installer for win$ARCH"

# Package steam windows tarball
OLDDIR=$(pwd)

cd $SOURCEROOT/extras/windows/installer/bin &&
tar -cJvf ../steam-win$ARCH.tar.xz *.dll *.exe
mv ../*.xz $OUTPUTPATH/out
cd $OLDDIR

echo "Successfully packaged Steam Tarball for win$ARCH"

echo "Cleaning up staging area"
rm -rf $SOURCEROOT/extras/windows/installer/bin
rm -rf $SOURCEROOT/extras/windows/installer/logo.ico
