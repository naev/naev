#!/bin/bash
# RELEASE SCRIPT FOR NAEV
#
# This script attempts to compile and build different parts of Naev
# automatically in order to prepare for a new release. 
#
# Pass in [-d] [-n] (set this for nightly builds) -s <SOURCEROOT> (Sets location of source) -b <BUILDROOT> (Sets location of build directory) -o <BUILDOUTPUT> (Dist output directory) -r <RUNNER> (must be specified)

# All outputs will be within pwd if nothing is passed in

set -e

while getopts dns:b:o:r: OPTION "$@"; do
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
    r)
        RUNNER="${OPTARG}"
        ;;
    esac
done

if [[ -z "$SOURCEROOT" ]]; then
    echo "usage: `basename $0` [-d] [-n] (set this for nightly builds) -s <SOURCEROOT> (Sets location of source) -b <BUILDROOT> (Sets location of build directory) -o <BUILDOUTPUT> (Dist output directory) -r <RUNNER> (must be specified)"
    exit 1
fi
if [[ -z "$BUILDPATH" ]]; then
    echo "usage: `basename $0` [-d] [-n] (set this for nightly builds) -s <SOURCEROOT> (Sets location of source) -b <BUILDROOT> (Sets location of build directory) -o <BUILDOUTPUT> (Dist output directory) -r <RUNNER> (must be specified)"
    exit 1
fi
if [[ $NIGHTLY = "true" ]]; then
    NIGHTLY="true"
else
    NIGHTLY="false"
fi
if [[ -z "$BUILDOUTPUT" ]]; then
    echo "usage: `basename $0` [-d] [-n] (set this for nightly builds) -s <SOURCEROOT> (Sets location of source) -b <BUILDROOT> (Sets location of build directory) -o <BUILDOUTPUT> (Dist output directory) -r <RUNNER> (must be specified)"
    exit 1
fi
if [[ -z "$RUNNER" ]]; then
    echo "usage: `basename $0` [-d] [-n] (set this for nightly builds) -s <SOURCEROOT> (Sets location of source) -b <BUILDROOT> (Sets location of build directory) -o <BUILDOUTPUT> (Dist output directory) -r <RUNNER> (must be specified)"
    exit 1
fi


function get_version {
   VERSION="$(cat $SOURCEROOT/dat/VERSION)"
   # Get version, negative minors mean betas
   if [[ -n $(echo "$VERSION") ]]; then
      VERSION=$VERSION
   else
      echo "could not find VERSION file"
      exit -1
   fi
   return 0
}

function make_appimage {
   sh "$SOURCEROOT/utils/buildAppImage.sh" -n -s "$SOURCEROOT" -b "$BUILDPATH/appimage" -o "$BUILDOUTPUT"
}

function make_windows {
   sh "$SOURCEROOT/extras/windows/packageWindows.sh" -n -s "$SOURCEROOT" -b "$BUILDPATH/windows" -o "$BUILDOUTPUT"
}

function make_macos {
   echo "WIP, this will be implemented in the near future."
}

function make_steam {
   if [[ $RUNNER == "Windows" ]]; then
      echo "TODO!"
   elif [[ $RUNNER == "macOS" ]]; then
      echo "Nothing to do!"
   elif [[ $RUNNER == "Linux" ]]; then
      echo "TODO!"
   else
      echo "Invalid Runner name, did you pass in runner.os?"
   fi
}

function make_itch {
   if [[ $RUNNER == "Windows" ]]; then
      echo "TODO!"
   elif [[ $RUNNER == "macOS" ]]; then
      echo "Nothing to do!"
   elif [[ $RUNNER == "Linux" ]]; then
      echo "TODO!"
   else
      echo "Invalid Runner name, did you pass in runner.os?"
   fi
}

# Create output dirdectory if necessary
mkdir -p "$BUILDOUTPUT"

# Build Release stuff
if [[ $RUNNER == "Windows" ]]; then
   make_windows
   make_steam
elif [[ $RUNNER == "macOS" ]]; then
   make_macos
elif [[ $RUNNER == "Linux" ]]; then
   make_appimage
   make_steam
else
   echo "Invalid Runner name, did you pass in runner.os?"
fi
