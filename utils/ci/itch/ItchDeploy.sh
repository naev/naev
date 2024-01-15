#!/bin/bash

set -e

usage() {
    cat <<EOF
ITCHIO DEPLOYMENT SCRIPT FOR NAEV

This script should be run after downloading all build artefacts
If -n is passed to the script, a nightly build will be generated
and uploaded to Itch.io

Pass in [-d] [-n] (set this for nightly builds) [-p] (set this for pre-release builds.) [-c] (set this for CI testing) -t <TEMPPATH> (itch.io build artefact location) -o <OUTDIR> (itch.io dist output directory)
EOF
    exit 1
}

# Defaults
NIGHTLY="false"
PRERELEASE="false"
TEMPPATH="$(pwd)"
OUTDIR="$(pwd)/dist"
DRYRUN="false"

while getopts dnpct:o: OPTION "$@"; do
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
    *)
        usage
        ;;
    esac
done

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

BUTLERDIR="butler-bin"

grab_butler () {
    local VERSION="LATEST"
    local PACKAGE="butler-linux-amd64.zip"

    if [ ! -d "$BUTLERDIR" ]; then
        mkdir "$BUTLERDIR"
    fi

    if [ ! -f "$BUTLER" ]; then
        echo "grabbing from online"
        curl -L "https://broth.itch.ovh/butler/linux-amd64/$VERSION/archive/default" -o $PACKAGE
        unzip $PACKAGE -d "$BUTLERDIR"
        rm $PACKAGE
    else
        echo "using cached version $VERSION"
    fi
}

if ! [ -x "$(command -v butler)" ]; then
    echo "You don't have butler in PATH"
    BUTLER="$BUTLERDIR/butler"
    grab_butler
else
    BUTLER="butler"
fi

run_butler () {
    if [[ "$BUTLER" = *"$BUTLERDIR"* ]]; then
        retry 5 "./$BUTLER" "$@"
    else
        retry 5 "$BUTLER" "$@"
    fi
}

# Collect the VERSION

VERSION="$(<"$TEMPPATH/naev-version/VERSION")"



# Make itch.io dist path if it does not exist
mkdir -p "$OUTDIR"/lin64
mkdir -p "$OUTDIR"/macos
mkdir -p "$OUTDIR"/win64

# Move all build artefacts to deployment locations
# Move Linux binary and set as executable
cp "$TEMPPATH"/naev-linux-x86-64/*.AppImage "$OUTDIR/lin64/naev-$VERSION-linux-x86-64.AppImage"
chmod +x "$OUTDIR/lin64/naev-$VERSION-linux-x86-64.AppImage"

# Move macOS bundle to deployment location
unzip "$TEMPPATH"/naev-macos/*.zip -d "$OUTDIR"/macos

# Unzip Windows binary and DLLs and move to deployment location
tar -Jxf "$TEMPPATH/naev-win64/naev-windows.tar.xz" -C "$OUTDIR/win64"
tar -Jxf "$TEMPPATH/naev-ndata/naev-ndata.tar.xz" -C "$OUTDIR/win64"

# Rename windows binary so it follows the correct naming scheme.
mv "$OUTDIR"/win64/*.exe "$OUTDIR/win64/naev-$VERSION-win64.exe"

# Prepare itch.toml for Linux

cp "$TEMPPATH"/naev-itch-deployment/.itch.toml "$OUTDIR"/lin64
sed -i "s/%EXECNAME%/naev-$VERSION-linux-x86-64.AppImage/" "$OUTDIR"/lin64/.itch.toml
sed -i 's/%PLATFORM%/linux/' "$OUTDIR"/lin64/.itch.toml

# Prepare itch.toml for macOS

cp "$TEMPPATH"/naev-itch-deployment/.itch.toml "$OUTDIR"/macos
sed -i 's/%EXECNAME%/Naev.app/' "$OUTDIR"/macos/.itch.toml
sed -i 's/%PLATFORM%/osx/' "$OUTDIR"/macos/.itch.toml

# Prepare itch.toml for Windows

cp "$TEMPPATH"/naev-itch-deployment/.itch.toml "$OUTDIR"/win64
sed -i "s/%EXECNAME%/naev-$VERSION-win64.exe/" "$OUTDIR"/win64/.itch.toml
sed -i 's/%PLATFORM%/windows/' "$OUTDIR"/win64/.itch.toml

# Prepare soundtrack
if [ "$NIGHTLY" == "false" ] && [ "$PRERELEASE" == "false" ]; then
    mkdir -p "$OUTDIR"/soundtrack
    unzip "$TEMPPATH"/naev-soundtrack/*.zip -d "$OUTDIR"/soundtrack
fi

# Push builds to itch.io via butler

if [ "$DRYRUN" == "false" ]; then
    run_butler -V
    if [ "$NIGHTLY" == "true" ]; then
        run_butler push --userversion="$VERSION" "$OUTDIR"/lin64 naev/naev:linux-x86-64-nightly
        run_butler push --userversion="$VERSION" "$OUTDIR"/macos naev/naev:macos-universal-nightly
        run_butler push --userversion="$VERSION" "$OUTDIR"/win64 naev/naev:windows-x86-64-nightly

    else
        if [ "$PRERELEASE" == "true" ]; then
            run_butler push --userversion="$VERSION" "$OUTDIR"/lin64 naev/naev:linux-x86-64-beta
            run_butler push --userversion="$VERSION" "$OUTDIR"/macos naev/naev:macos-universal-beta
            run_butler push --userversion="$VERSION" "$OUTDIR"/win64 naev/naev:windows-x86-64-beta

        elif [ "$PRERELEASE" == "false" ]; then
            run_butler push --userversion="$VERSION" "$OUTDIR"/lin64 naev/naev:linux-x86-64
            run_butler push --userversion="$VERSION" "$OUTDIR"/macos naev/naev:macos-universal
            run_butler push --userversion="$VERSION" "$OUTDIR"/win64 naev/naev:windows-x86-64
            run_butler push --userversion="$VERSION" "$OUTDIR"/soundtrack naev/naev:soundtrack

        else
            echo "Something went wrong determining if this is a PRERELEASE or not."
        fi
    fi
elif [ "$DRYRUN" == "true" ]; then
    run_butler -V
    if [ "$NIGHTLY" == "true" ]; then
        # Run butler nightly upload
        echo "butler nightly upload"
        ls -l -R "$OUTDIR"
    else
        if [ "$PRERELEASE" == "true" ]; then
            # Run butler beta upload
            echo "butler beta upload"
            ls -l -R "$OUTDIR"
        elif [ "$PRERELEASE" == "false" ]; then
            # Run butler release upload
            echo "butler release upload"
            echo "butler soundtrack upload"
            ls -l -R "$OUTDIR"

        else
            echo "Something went wrong determining if this is a PRERELEASE or not."
        fi
    fi

else
    echo "Something went wrong determining which mode to run this script in."
    exit 1
fi
