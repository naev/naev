#!/bin/bash
# WINDOWS PACKAGING SCRIPT FOR NAEV
# Requires NSIS, and python3-pip to be installed
#
# This script should be run after compiling Naev
# It detects the current environment, and builds the appropriate NSIS installer
# into the root naev directory.
#
# Pass in [-d] [-n] (set this for nightly builds) -s <SOURCEROOT> (Sets location of source) -o <OUTPUTPATH> (dist output directory)

set -e

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

# Checks if argument(s) are valid

if [[ -z "$SOURCEROOT" ]]; then
    SOURCEROOT=$(pwd)
fi
if [[ -z "$BUILDDIR" ]]; then
    BUILDDIR=$(pwd)/build
fi
if [[ $NIGHTLY = "true" ]]; then
    NIGHTLY="true"
    BUILD_DATE="$(date +%m%d%Y)"
else
    NIGHTLY="false"
fi
if [[ -z "$OUTPUTPATH" ]]; then
    OUTPUTPATH="$(pwd)/dist"
fi

# Output configured variables

echo "SOURCE ROOT:  $SOURCEROOT"
echo "BUILD ROOT:   $BUILDDIR"
if [[ $NIGHTLY = "true" ]]; then
    echo "NIGHTLY:      YES"
else
    echo "NIGHTLY:      NO"
fi
echo "BUILD OUTPUT: $OUTPUTPATH"

# Rudementary way of detecting which environment we are packaging.. 
# It works (tm), and it should remain working until msys changes their naming scheme

if [[ $PATH == *"mingw32"* ]]; then
    echo "Detected MinGW32 environment"
    ARCH="32"
elif [[ $PATH == *"mingw64"* ]]; then
    echo "Detected MinGW64 environment"
    ARCH="64"
else
    echo "Welp, I don't know what environment this is... Make sure you are running this in an MSYS2 MinGW environment"
    exit -1
fi

echo "ARCH:      $ARCH"

VERSION="$(cat $SOURCEROOT/dat/VERSION)"
BETA="false"
# Get version, negative minors mean betas
if [[ -n $(echo "$VERSION") ]]; then
    VERSION=$VERSION
    if [[ -n $(echo "$VERSION" | grep "beta") ]]; then
    BETA="true"
    fi
else
    echo "could not find VERSION file"
    exit -1
fi

echo "BETA:      $BETA"

# Download and Install mingw-ldd

echo "Update pip"
pip3 install --upgrade pip

echo "Install mingw-ldd script"
pip3 install mingw-ldd

# Move compiled binary to staging folder.

echo "creating staging area"
mkdir -p $SOURCEROOT/extras/windows/installer/bin

# Move data to staging folder
echo "moving data to staging area"
cp -r $SOURCEROOT/dat $SOURCEROOT/extras/windows/installer/bin

# Collect DLLs
 
if [[ $ARCH == "32" ]]; then
for fn in `mingw-ldd "$BUILDDIR/naev.exe" --dll-lookup-dirs /mingw32/bin | grep -i "mingw32" | cut -f1 -d"/" --complement | cut -f1 -d"/" --complement`; do
    fp="/"$fn
    echo "copying $fp to staging area"
    cp $fp $SOURCEROOT/extras/windows/installer/bin
done
elif [[ $ARCH == "64" ]]; then
for fn in `mingw-ldd "$BUILDDIR/naev.exe" --dll-lookup-dirs /mingw64/bin | grep -i "mingw64" | cut -f2 -d"/" --complement | cut -f1 -d"/" --complement`; do
    fp="/"$fn
    echo "copying $fp to staging area"
    cp $fp $SOURCEROOT/extras/windows/installer/bin
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
cp $BUILDDIR/naev.exe $SOURCEROOT/extras/windows/installer/bin/naev-$VERSION-$BUILD_DATE-win$ARCH.exe
elif [[ $NIGHTLY == false ]]; then
cp $BUILDDIR/naev.exe $SOURCEROOT/extras/windows/installer/bin/naev-$VERSION-win$ARCH.exe
else
    echo "Cannot think of another movie quote."
    echo "Something went wrong while copying binary to staging area."
    exit -1
fi

# Create distribution folder

echo "creating distribution folder"
mkdir -p $OUTPUTPATH/release

# Build installer

if [[ $NIGHTLY = true ]]; then
    if [[ $BETA = true ]]; then 
        makensis -DVERSION=$VERSION.$BUILD_DATE -DARCH=$ARCH $SOURCEROOT/extras/windows/installer/naev.nsi
    elif [[ $BETA = false ]]; then 
        makensis -DVERSION=$VERSION.$BUILD_DATE -DARCH=$ARCH $SOURCEROOT/extras/windows/installer/naev.nsi
    else
        echo "Something went wrong determining if this is a beta or not."
    fi
    

    # Move installer to distribution directory
    mv $SOURCEROOT/extras/windows/installer/naev-$VERSION.$BUILD_DATE-win$ARCH.exe $OUTPUTPATH/release

elif [[ $NIGHTLY == false ]]; then
    if [[ $BETA = true ]]; then 
        makensis -DVERSION=$VERSION -DARCH=$ARCH $SOURCEROOT/extras/windows/installer/naev.nsi
    elif [[ $BETA = false ]]; then 
        makensis -DVERSION=$VERSION -DARCH=$ARCH $SOURCEROOT/extras/windows/installer/naev.nsi
    else
        echo "Something went wrong determining if this is a beta or not."
    fi

    # Move installer to distribution directory
    mv $SOURCEROOT/extras/windows/installer/naev-$VERSION-win$ARCH.exe $OUTPUTPATH/release
else
    echo "Cannot think of another movie quote.. again."
    echo "Something went wrong.."
    exit -1
fi

echo "Successfully built Windows Installer for win$ARCH"

# Package zip
OLDDIR=$(pwd)

cd $SOURCEROOT/extras/windows/installer/bin
zip $OUTPUTPATH/release/naev-win$ARCH.zip *.dll *.exe
cd $OLDDIR

echo "Successfully packaged zipped folder for win$ARCH"

echo "Cleaning up staging area"
rm -rf $SOURCEROOT/extras/windows/installer/bin
rm -rf $SOURCEROOT/extras/windows/installer/logo.ico
