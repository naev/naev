#!/bin/bash

# STEAM DEPLOYMENT SCRIPT FOR NAEV
# Requires SteamCMD to be installed within a Github Actions ubuntu-latest runner.
#
# Written by Jack Greiner (ProjectSynchro on Github: https://github.com/ProjectSynchro/) 
#
# This script should be run after downloading all build artefacts
# If --nightly is passed to the script, a nightly build will be generated
# If --soundtrack is passed to the script, the soundtrack will be generated
# and uploaded to Steam
#

# Checks if argument(s) are valid

if [[ $1 == "--nightly" ]]; then
    echo "Deploying for nightly release"
    NIGHTLY=true
elif [[ $1 == "" ]]; then
    echo "No arguments passed, assuming normal release"
    NIGHTLY=false
elif [[ $1 != "--nightly" ]]; then
    echo "Please use argument --nightly if you are deploying this as a nightly release"
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

# Create deployment locations
mkdir -p extras/steam/{content,output}
mkdir -p extras/steam/content/{lin32,lin64,ndata,win32,win64,soundtrack}

# Move all build artefacts to deployment locations

# Move Linux binaries and set as executable
mv extras/steam/temp/steam-x86-64/naev.x64 extras/steam/content/lin64/naev.x64
chmod +x extras/steam/content/lin64/naev.x64

mv extras/steam/temp/steam-x86-32/naev.x32 extras/steam/content/lin32/naev.x32
chmod +x extras/steam/content/lin32/naev.x32
          
# Will figure this out soon TM. (might need a separate mac depot with all of the files pre-assembled.)
# unzip extras/steam/temp/macos/naev-macos.zip -d extras/steam/content/macos/

# Unzip Windows binaries and DLLs and move to deployment locations
unzip extras/steam/temp/win64/naev-win64.zip -d extras/steam/content/win64/
mv extras/steam/content/win64/naev*.exe extras/steam/content/win64/naev.exe

unzip extras/steam/temp/win32/naev-win32.zip -d extras/steam/content/win32/
mv extras/steam/content/win32/naev*.exe extras/steam/content/win32/naev.exe

#Unzip ndata and move to deployment location
unzip extras/steam/temp/ndata/ndata.zip -d extras/steam/content/ndata

# Runs STEAMCMD, and builds the app as well as all needed depots.

if [[ $NIGHTLY == true ]]; then
    # Trigger 2FA request and get 2FA code
    steamcmd +login $STEAMCMD_USER $STEAMCMD_PASS +quit || true

    # Wait a few seconds for the email to arrive
    sleep 10
    python3 extras/steam/2fa/get_2fa.py
    STEAMCMD_TFA="$(cat extras/steam/2fa/2fa.txt)"

    # Run steam upload with 2fa key
    steamcmd +login $STEAMCMD_USER $STEAMCMD_PASS $STEAMCMD_TFA +run_app_build_http /home/runner/work/naev/naev/extras/steam/scripts/app_build_598530_nightly.vdf +quit

elif [[ $NIGHTLY == false ]]; then
    # Move soundtrack stuff to deployment area
    unzip extras/steam/temp/soundtrack/soundtrack.zip -d extras/steam/content/soundtrack
    cp extras/steam/naev_soundtrack_cover.png extras/steam/content/soundtrack/naev_soundtrack_cover.png

    # Trigger 2FA request and get 2FA code
    steamcmd +login $STEAMCMD_USER $STEAMCMD_PASS +quit || true

    # Wait a few seconds for the email to arrive
    sleep 10
    python3 extras/steam/2fa/get_2fa.py
    STEAMCMD_TFA="$(cat extras/steam/2fa/2fa.txt)"

    if [[ $BETA == true ]]; then 
        # Run steam upload with 2fa key
        steamcmd +login $STEAMCMD_USER $STEAMCMD_PASS $STEAMCMD_TFA +run_app_build_http /home/runner/work/naev/naev/extras/steam/scripts/app_build_598530_prerelease.vdf +quit

    elif [[ $BETA == false ]]; then 
        # Run steam upload with 2fa key
        steamcmd +login $STEAMCMD_USER $STEAMCMD_PASS $STEAMCMD_TFA +run_app_build_http /home/runner/work/naev/naev/extras/steam/scripts/app_build_598530_release.vdf +quit
        steamcmd +login $STEAMCMD_USER $STEAMCMD_PASS $STEAMCMD_TFA +run_app_build_http /home/runner/work/naev/naev/extras/steam/scripts/app_build_1411430.vdf +quit

    else
        echo "Something went wrong determining if this is a beta or not."
    fi
elif [[ $SOUNDTRACK == true ]]; then
else
    echo "Something went wrong.."
    exit -1
fi
