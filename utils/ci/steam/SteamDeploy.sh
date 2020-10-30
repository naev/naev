#!/bin/bash

# STEAM DEPLOYMENT SCRIPT FOR NAEV
# Requires SteamCMD to be installed within a Github Actions ubuntu-latest runner.
#
# Written by Jack Greiner (ProjectSynchro on Github: https://github.com/ProjectSynchro/) 
#
# This script should be run after downloading all build artefacts
# If -n is passed to the script, a nightly build will be generated
# and uploaded to Steam
#
# Pass in [-d] [-n] (set this for nightly builds) -s <SOURCEROOT> (Sets location of source) -o <STEAMPATH> (Steam dist output directory)

set -e

while getopts dns:o: OPTION "$@"; do
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
    o)
        STEAMPATH="${OPTARG}"
        ;;
    esac
done

# Checks if argument(s) are valid

if [[ $NIGHTLY = "true" ]]; then
    NIGHTLY="true"
else
    NIGHTLY="false"
fi

VERSION="$(cat $SOURCEROOT/dat/VERSION)"
BETA=false

# Get version, negative minors mean betas
if [[ -n $(echo "$VERSION" | grep "beta") ]]; then
    BETA=true
else
    echo "could not find VERSION file"
    exit -1
fi

# Move Depot Scripts to Steam staging area
cp $SOURCEROOT/utils/ci/steam/scripts $STEAMPATH

# Move all build artefacts to deployment locations

# Move Linux binary and set as executable
mv naev-steamruntime/naev.x64 $STEAMPATH/content/lin64/naev.x64
chmod +x $STEAMPATH/content/lin64/naev.x64
          
# Move macOS bundle to deployment location (Bye Bye for now)
# unzip naev-macOS/naev-macos.zip -d $STEAMPATH/content/macos/

# Unzip Windows binary and DLLs and move to deployment location
tar -Jxvf naev-windows-latest/steam-win64.tar.xz $STEAMPATH/content/win64/
mv $STEAMPATH/content/win64/naev*.exe $STEAMPATH/content/win64/naev.exe

# Move data to deployment location
cp -r $SOURCEROOT/dat $STEAMPATH/content/ndata

tree $STEAMPATH