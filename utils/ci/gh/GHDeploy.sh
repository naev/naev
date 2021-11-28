#!/bin/bash

# GITHUB DEPLOYMENT SCRIPT FOR NAEV
#
# This script should be run after downloading all build artefacts.
# It also makes use of ENV variable GH_TOKEN to login. ensure this is exported.
#
#
# Pass in [-d] [-n] (set this for nightly builds) [-p] (set this for pre-release builds.) [-c] (set this for CI testing) -t <TEMPPATH> (build artefact location) -o <OUTDIR> (dist output directory) -r <TAGNAME> (tag of release *required*) -g <REPONAME> (defaults to naev/naev)

set -e

# Defaults
NIGHTLY="false"
PRERELEASE="false"
REPO="naev/naev"
TEMPPATH="$(pwd)"
OUTDIR="$(pwd)/dist"
DRYRUN="false"
REPONAME="naev/naev"

while getopts dnpct:o:r:g: OPTION "$@"; do
    case $OPTION in
    d)
        set -x
        ;;
    n)
        NIGHTLY="true"
        ;;
    p)
        PRERELEASE="true"
        ;;
    c)
        DRYRUN="true"
        ;;
    t)
        TEMPPATH="${OPTARG}"
        ;;
    o)
        OUTDIR="${OPTARG}"
        ;;
    r)
        TAGNAME="${OPTARG}"
        ;;
    g)
        REPONAME="${OPTARG}"
        ;;
    esac
done

if [[ -z "$TAGNAME" ]]; then
    echo "usage: `basename $0` [-d] [-n] (set this for nightly builds) [-p] (set this for pre-release builds.) [-c] (set this for CI testing) -t <TEMPPATH> (build artefact location) -o <OUTDIR> (dist output directory) -r <TAGNAME> (tag of release *required*) -g <REPONAME> (defaults to naev/naev)"
    exit -1
fi

retry() {
    local -r -i max_attempts="$1"; shift
    local -i attempt_num=1
    until "$@"
    do
        if ((attempt_num==max_attempts))
        then
            echo "Attempt $attempt_num failed and there are no more attempts left!"
            return 1
        else
            echo "Attempt $attempt_num failed! Trying again in $attempt_num seconds..."
            sleep $((attempt_num++))
        fi
    done
}

if ! [ -x "$(command -v gh)" ]; then
    echo "You don't have gh (github cli client) in PATH"
    exit -1
else
    GH="gh"
fi

run_gh () {
    retry 5 $GH $@
}

# Collect date and assemble the VERSION suffix

BUILD_DATE="$(date +%Y%m%d)"
VERSION="$(<"$TEMPPATH/naev-version/VERSION")"

if [ "$NIGHTLY" == "true" ]; then
    SUFFIX="$VERSION+DEBUG.$BUILD_DATE"
else
    SUFFIX="$VERSION"
fi


# Make dist path if it does not exist
mkdir -p "$OUTDIR"/dist
mkdir -p "$OUTDIR"/lin64
mkdir -p "$OUTDIR"/macos
mkdir -p "$OUTDIR"/win64
mkdir -p "$OUTDIR"/soundtrack

# Move all build artefacts to deployment locations
# Move Linux binary and set as executable
cp "$TEMPPATH"/naev-linux-x86-64/*.AppImage "$OUTDIR"/lin64/naev-$SUFFIX-linux-x86-64.AppImage
chmod +x "$OUTDIR"/lin64/naev-$SUFFIX-linux-x86-64.AppImage

# Move macOS bundle to deployment location
cp "$TEMPPATH"/naev-macos/*.zip -d "$OUTDIR"/macos/naev-$SUFFIX-macos.zip

# Move Windows installer to deployment location
cp "$TEMPPATH"/naev-win64/naev*.exe "$OUTDIR"/win64/naev-$SUFFIX-win64.exe

# Move Dist to deployment location
cp "$TEMPPATH"/naev-dist/source.tar.xz "$OUTDIR"/dist/naev-$SUFFIX-source.tar.xz

# Move Soundtrack to deployment location if this is a release.
if [ "$NIGHTLY" == "true" ] || [ "$PRERELEASE" == "true" ]; then
    echo "not preparing soundtrack"
else
    cp "$TEMPPATH"/naev-soundtrack/naev-*-soundtrack.zip "$OUTDIR"/dist/naev-$SUFFIX-soundtrack.zip
fi

# Push builds to github via gh

if [ "$DRYRUN" == "false" ]; then
    run_gh --version
    run_gh release upload "$TAGNAME" "$OUTDIR"/lin64/* --clobber
    run_gh release upload "$TAGNAME" "$OUTDIR"/macos/* --clobber
    run_gh release upload "$TAGNAME" "$OUTDIR"/win64/* --clobber
    if [ "$NIGHTLY" == "false" ] && [ "$PRERELEASE" == "false" ]; then
        run_gh release upload "$TAGNAME" "$OUTDIR"/soundtrack/* --clobber
    fi
    run_gh release upload "$TAGNAME" "$OUTDIR"/dist/* --clobber
elif [ "$DRYRUN" == "true" ]; then
    run_gh --version
    if [ "$NIGHTLY" == "true" ]; then
        echo "github nightly upload"
    elif [ "$PRERELEASE" == "true" ]; then
        echo "github beta upload"
    else
        echo "github release upload"
        echo "github soundtrack upload"
    fi
    ls -l -R "$OUTDIR"
else
    echo "Something went wrong determining which mode to run this script in."
    exit 1
fi
