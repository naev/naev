#!/bin/bash
# WINDOWS PACKAGING SCRIPT FOR NAEV
# Requires NSIS, and python3-pip to be installed
#
# This script should be run after compiling Naev
# It detects the current environment, and builds the appropriate NSIS installer
# into the root naev directory.
#

# Checks if argument(s) are valid

if [[ $1 == "--nightly" ]]; then
    echo "Building for nightly release"
    NIGHTLY=true
    # Get Formatted Date
    BUILD_DATE="$(date +%m_%d_%Y)"
elif [[ $1 == "" ]]; then
    echo "No arguments passed, assuming normal release"
    NIGHTLY=false
elif [[ $1 != "--nightly" ]]; then
    echo "Please use argument --nightly if you are building this as a nightly build"
    exit -1
else
    echo "Something went wrong."
    exit -1
fi

# Check if we are running in the right place

if [[ ! -f "naev.6" ]]; then
    echo "Please run from Naev root directory."
    exit -1
fi

# Rudementary way of detecting which environment we are packaging.. 
# It works, and it should remain working until msys changes their naming scheme

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

VERSION="$(cat $(pwd)/VERSION)"
BETA=false
# Get version, negative minors mean betas
if [[ -n $(echo "$VERSION" | grep "-") ]]; then
    BASEVER=$(echo "$VERSION" | sed 's/\.-.*//')
    BETAVER=$(echo "$VERSION" | sed 's/.*-//')
    VERSION="$BASEVER.0-beta.$BETAVER"
    BETA=true
else
    echo "could not find VERSION file"
    exit -1
fi

# Download and Install mingw-ldd

echo "Update pip"
pip3 install --upgrade pip

echo "Install mingw-ldd script"
pip3 install mingw-ldd

# Move compiled binary to staging folder.

echo "creating staging area"
mkdir -p extras/windows/installer/bin

# Collect DLLs
 
if [[ $ARCH == "32" ]]; then
for fn in `mingw-ldd naev.exe --dll-lookup-dirs /mingw32/bin | grep -i "mingw32" | cut -f1 -d"/" --complement`; do
    fp="/"$fn
    echo "copying $fp to staging area"
    cp $fp extras/windows/installer/bin
done
elif [[ $ARCH == "64" ]]; then
for fn in `mingw-ldd naev.exe --dll-lookup-dirs /mingw64/bin | grep -i "mingw64" | cut -f1 -d"/" --complement`; do
    fp="/"$fn
    echo "copying $fp to staging area"
    cp $fp extras/windows/installer/bin
done
else
    echo "Aw, man, I shot Marvin in the face..."
    echo "Something went wrong while looking for DLLs to stage."
    exit -1
fi

echo "copying naev binary to staging area"
if [[ $NIGHTLY == true ]]; then
cp src/naev.exe extras/windows/installer/bin/naev-$VERSION-$BUILD_DATE-win$ARCH.exe
elif [[ $NIGHTLY == false ]]; then
cp src/naev.exe extras/windows/installer/bin/naev-$VERSION-win$ARCH.exe
else
    echo "Cannot think of another movie quote."
    echo "Something went wrong while copying binary to staging area."
    exit -1
fi

# Create distribution folder

echo "creating distribution folder"
mkdir -p dist/release

# Build installer

if [[ $NIGHTLY == true ]]; then
    if [[ $BETA == true ]]; then 
        makensis -DVERSION=$BASEVER.0 -DVERSION_SUFFIX=-beta.$BETAVER-$BUILD_DATE -DARCH=$ARCH extras/windows/installer/naev.nsi
    elif [[ $BETA == false ]]; then 
        makensis -DVERSION=$VERSION -DVERSION_SUFFIX=-$BUILD_DATE -DARCH=$ARCH extras/windows/installer/naev.nsi
    else
        echo "Something went wrong determining if this is a beta or not."
    fi
    

# Move installer to distribution directory
mv extras/windows/installer/naev-$VERSION-$BUILD_DATE-win$ARCH.exe dist/release/naev-win$ARCH.exe

elif [[ $NIGHTLY == false ]]; then
    if [[ $BETA == true ]]; then 
        makensis -DVERSION=$BASEVER.0 -DVERSION_SUFFIX=-beta.$BETAVER -DARCH=$ARCH extras/windows/installer/naev.nsi
    elif [[ $BETA == false ]]; then 
        makensis -DVERSION=$VERSION -DVERSION_SUFFIX= -DARCH=$ARCH extras/windows/installer/naev.nsi
    else
        echo "Something went wrong determining if this is a beta or not."
    fi

# Move installer to distribution directory
mv extras/windows/installer/naev-$VERSION-win$ARCH.exe dist/release/naev-win$ARCH.exe
else
    echo "Cannot think of another movie quote.. again."
    echo "Something went wrong.."
    exit -1
fi

echo "Successfully built Windows Installer for win$ARCH"

# Package zip

if [[ $NIGHTLY == true ]]; then
cd extras/windows/installer/bin
zip ../../../../dist/release/naev-win$ARCH.zip *.*
cd ../../../../dist/release/

elif [[ $NIGHTLY == false ]]; then
cd extras/windows/installer/bin
zip ../../../../dist/release/naev-win$ARCH.zip *.*
cd ../../../../dist/release/

else
    echo "Cannot think of another movie quote.. again."
    echo "Something went wrong.."
    exit -1
fi

echo "Successfully packaged zipped folder for win$ARCH"

echo "Cleaning up staging area"
rm -rf extras/windows/installer/bin
