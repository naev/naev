#!/bin/bash

set -e

usage() {
    echo "usage: $(basename "$0") [-d] [-n] (set this for nightly builds) -s <SOURCEROOT> (Sets location of source) -b <BUILDROOT> (Sets location of build directory) -r <RUNNER> (must be specified)"
    cat <<EOF
DMG Packaging Script for Naev

This should be run after meson install -C build, and can be run from the source root. Requires genisoimage

Pass in [-d] [-s] (Sets location of source root) [-b] <BUILDPATH> (Sets location of meson build directory)
EOF
    exit 1
}

# Defaults
SOURCEPATH="$(pwd)"
BUILDPATH="$(pwd)/build"

while getopts ds:b: OPTION "$@"; do
    case $OPTION in
    d)
        set -x
        ;;
    s)
        SOURCEPATH="${OPTARG}"
        ;;
    b)
        BUILDPATH="${OPTARG}"
        ;;
    *)
        usage
        ;;
    esac
done

if ! [ -x "$(command -v genisoimage)" ]; then
    echo "You don't have genisoimage in PATH"
    exit 1
fi


# Creates temp directory
WORKPATH=$(readlink -mf "$(mktemp -d)")

# Make temp directories
mkdir -p "$WORKPATH/{BundleDir}"

# Copy all DMG assets to BundleDir
cp -r "$SOURCEPATH"/extras/macos/dmg_assets/* "$WORKPATH"/BundleDir

# Extract Naev app bundle to BundleDir
cp -r "$BUILDPATH"/dist/Naev.app "$WORKPATH"/BundleDir

# Generate DMG image
genisoimage -V Naev -D -R -apple -no-pad -o "$BUILDPATH"/dist/naev.dmg "$WORKPATH"/BundleDir
