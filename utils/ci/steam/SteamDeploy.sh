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
# Pass in [-d] [-n] (set this for nightly builds) [-n] (set this for CI testing) -v <VERSIONPATH> (Sets path of the VERSION file.) -s <SCRIPTROOT> (Sets path to look for additional steam scripts.) -t <TEMPPATH> (Steam build artefact location) -o <STEAMPATH> (Steam dist output directory)

set -e

# Defaults
NIGHTLY="false"
BETA="false"
SCRIPTROOT="$(pwd)"
DRYRUN="false"

while getopts dncv:s:t:o: OPTION "$@"; do
    case $OPTION in
    d)
        set -x
        ;;
    n)
        NIGHTLY="true"
        ;;
    c)
        DRYRUN="true"
        ;;
    v)
        VERSIONPATH="${OPTARG}"
        ;;
    s)
        SCRIPTROOT="${OPTARG}"
        ;;
    t)
        TEMPPATH="${OPTARG}"
        ;;
    o)
        STEAMPATH="${OPTARG}"
        ;;
    esac
done

if [ -f "$VERSIONPATH/VERSION" ]; then
    export VERSION="$(<"$VERSIONPATH/VERSION")"
else
    echo "The VERSION file is missing from $VERSIONPATH."
    exit 1
fi

# Get version, negative minors mean betas
if [ -n $(echo "$VERSION" | grep "beta") ]; then
    BETA="true"
else
    echo "could not find VERSION file"
    exit 1
fi

# Make Steam dist path if it does not exist
mkdir -p "$STEAMPATH"/content/lin64
mkdir -p "$STEAMPATH"/content/win64
mkdir -p "$STEAMPATH"/content/ndata

# Move Depot Scripts to Steam staging area
cp -v -r "$SCRIPTROOT"/scripts "$STEAMPATH"

# Move all build artefacts to deployment locations
# Move Linux binary and set as executable
cp -v "$TEMPPATH"/naev-steamruntime/naev.x64 "$STEAMPATH"/content/lin64
chmod +x "$STEAMPATH"/content/lin64/naev.x64
          
# Move macOS bundle to deployment location (Bye Bye for now)
# unzip "$TEMPPATH/naev-macOS/naev-macos.zip" -d "$STEAMPATH/content/macos/"

# Unzip Windows binary and DLLs and move to deployment location
tar -Jxf "$TEMPPATH/naev-win64/steam-win64.tar.xz" -C "$STEAMPATH/content/win64"
mv "$STEAMPATH"/content/win64/naev*.exe "$STEAMPATH/content/win64/naev.exe"

# Move data to deployment location
tar -Jxf "$TEMPPATH/naev-ndata/steam-ndata.tar.xz" -C "$STEAMPATH/content/ndata"

# Runs STEAMCMD, and builds the app as well as all needed depots.

if [ "$DRYRUN" == "false" ]; then

    # Trigger 2FA request and get 2FA code
    steamcmd +login $STEAMCMD_USER $STEAMCMD_PASS +quit || true

    # Wait a bit for the email to arrive
    sleep 60s
    python3 "$SCRIPTROOT/2fa/get_2fa.py"
    STEAMCMD_TFA="$(<"$SCRIPTROOT/2fa/2fa.txt")"

    if [ "$NIGHTLY" == "true" ]; then
        # Run steam upload with 2fa key
        steamcmd +login $STEAMCMD_USER $STEAMCMD_PASS $STEAMCMD_TFA +run_app_build_http "$STEAMPATH/scripts/app_build_598530_nightly.vdf" +quit
    else
        if [ "$BETA" == "true" ]; then 
            # Run steam upload with 2fa key
            steamcmd +login $STEAMCMD_USER $STEAMCMD_PASS $STEAMCMD_TFA +run_app_build_http "$STEAMPATH/scripts/app_build_598530_prerelease.vdf" +quit

        elif [ "$BETA" == "false" ]; then 
            # Move soundtrack stuff to deployment area
            cp "$TEMPPATH"/naev-steam-soundtrack/*.mp3 "$STEAMPATH/content/soundtrack"
            cp "$TEMPPATH"/naev-steam-soundtrack/*.png "$STEAMPATH/content/soundtrack"

            # Run steam upload with 2fa key
            steamcmd +login $STEAMCMD_USER $STEAMCMD_PASS $STEAMCMD_TFA +run_app_build_http "$STEAMPATH/scripts/app_build_598530_release.vdf" +quit
            steamcmd +login $STEAMCMD_USER $STEAMCMD_PASS +run_app_build_http "$STEAMPATH/scripts/app_build_1411430_soundtrack.vdf" +quit

        else
            echo "Something went wrong determining if this is a beta or not."
        fi
    fi
elif [ "$DRYRUN" == "true" ]; then
    # Trigger 2FA request and get 2FA code
    echo "steamcmd login"

    #Wait a bit for the email to arrive
    echo "2fa script"

    if [ "$NIGHTLY" == "true" ]; then
        # Run steam upload with 2fa key
        echo "steamcmd nightly build"
        ls -l -R "$STEAMPATH"
    else
        if [ "$BETA" == "true" ]; then 
            # Run steam upload with 2fa key
            echo "steamcmd beta build"
            ls -l -R "$STEAMPATH"
        elif [ "$BETA" == "false" ]; then 
            # Move soundtrack stuff to deployment area
            cp -v "$TEMPPATH"/naev-steam-soundtrack/*.mp3 "$STEAMPATH/content/soundtrack"
            cp -v "$TEMPPATH"/naev-steam-soundtrack/*.png "$STEAMPATH/content/soundtrack"

            # Run steam upload with 2fa key
            echo "steamcmd release build"
            echo "steamcmd soundtrack build"
            ls -l -R "$STEAMPATH"

        else
            echo "Something went wrong determining if this is a beta or not."
        fi
    fi

else
    echo "Something went wrong determining which mode to run this script in."
    exit 1
fi
