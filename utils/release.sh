#!/bin/bash
# RELEASE SCRIPT FOR NAEV Soon (tm)
#
# This script attempts to compile and build different parts of Naev
# automatically in order to prepare for a new release. 
#
# Pass in [-d] [-n] (set this for nightly builds) -s <SOURCEROOT> (Sets location of source) -b <BUILDROOT> (Sets location of build directory) -o <BUILDOUTPUT> (Dist output directory) -r <RUNNER> (must be specified)

# All outputs will be within pwd if nothing is passed in

set -e

# Defaults
NIGHTLY="false"

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

if [[ -z "$SOURCEROOT" || -z "$BUILDPATH" || -z "$BUILDOUTPUT" || -z "$RUNNER" ]]; then
    echo "usage: `basename $0` [-d] [-n] (set this for nightly builds) -s <SOURCEROOT> (Sets location of source) -b <BUILDROOT> (Sets location of build directory) -o <BUILDOUTPUT> (Dist output directory) -r <RUNNER> (must be specified)"
    exit 1
fi


function get_version {
   if [ -f "$SOURCEROOT/dat/VERSION" ]; then
       export VERSION="$(<"$SOURCEROOT/dat/VERSION")"
   else
       echo "The VERSION file is missing from $SOURCEROOT."
       exit -1
   fi

   return 0
}

function make_appimage {
   if [[ "$NIGHTLY" == "true" ]]; then
      sh "$SOURCEROOT/utils/buildAppImage.sh" -n -s "$SOURCEROOT" -b "$BUILDPATH/appimage" -o "$BUILDOUTPUT"
   else
      sh "$SOURCEROOT/utils/buildAppImage.sh" -s "$SOURCEROOT" -b "$BUILDPATH/appimage" -o "$BUILDOUTPUT"
   fi
}

function make_windows {
   if [[ "$NIGHTLY" == "true" ]]; then
      sh "$SOURCEROOT/extras/windows/packageWindows.sh" -n -s "$SOURCEROOT" -b "$BUILDPATH" -o "$BUILDOUTPUT"
   else
      sh "$SOURCEROOT/extras/windows/packageWindows.sh" -s "$SOURCEROOT" -b "$BUILDPATH" -o "$BUILDOUTPUT"
   fi
}

function make_macos {
   if [[ "$NIGHTLY" == "true" ]]; then
      "$SOURCEROOT/extras/macos/bundle.py" -n -s "$SOURCEROOT" -b "$BUILDPATH" -o "$BUILDOUTPUT"
   else
      "$SOURCEROOT/extras/macos/bundle.py" -s "$SOURCEROOT" -b "$BUILDPATH" -o "$BUILDOUTPUT"
   fi
   echo "WIP, remember to install a suitable dat/ in Contents/Resources. Don't forget dat/gettext."
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
