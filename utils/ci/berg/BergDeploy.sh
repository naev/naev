#!/bin/bash

# CODEBERG STAGING SCRIPT FOR NAEV
# This script should be run after downloading all build artefacts.
# It prepares the output directories and copies/moves the build artefacts for upload via Forgejo/CD actions.
#
# Pass in [-d] [-n] (set this for nightly builds) [-p] (set this for pre-release builds.) [-c] (set this for CI testing) -t <TEMPPATH> (build artefact location) -o <OUTDIR> (dist output directory) -r <TAGNAME> (tag of release *required*)

set -e

# Defaults
NIGHTLY="false"
PRERELEASE="false"
TEMPPATH="$(pwd)"
OUTDIR="$(pwd)/dist"


while getopts dnpt:o:r: OPTION "$@"; do
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
   t)
      TEMPPATH="${OPTARG}"
      ;;
   o)
      OUTDIR="${OPTARG}"
      ;;
   r)
      TAGNAME="${OPTARG}"
      ;;
   *)
      ;;
   esac
   done

if [[ -z "$TAGNAME" ]]; then
   echo "usage: $(basename "$0") [-d] [-n] (set this for nightly builds) [-p] (set this for pre-release builds.) [-c] (set this for CI testing) -t <TEMPPATH> (build artefact location) -o <OUTDIR> (dist output directory) -r <TAGNAME> (tag of release *required*)"
   exit 1
fi

VERSION="$(<"$TEMPPATH/naev-version/VERSION")"

# Make dist path if it does not exist
mkdir -p "$OUTDIR"/dist
mkdir -p "$OUTDIR"/lin64
mkdir -p "$OUTDIR"/macos
mkdir -p "$OUTDIR"/win64
mkdir -p "$OUTDIR"/soundtrack

# Move all build artefacts to deployment locations
# Move Linux AppImage, zsync files and set AppImage as executable
cp "$TEMPPATH"/naev-linux-x86-64/*.AppImage "$OUTDIR"/lin64/naev-"$VERSION"-linux-x86-64.AppImage
cp "$TEMPPATH"/naev-linux-x86-64/*.zsync "$OUTDIR"/lin64/naev-"$VERSION"-linux-x86-64.AppImage.zsync

chmod +x "$OUTDIR"/lin64/naev-"$VERSION"-linux-x86-64.AppImage

# Move macOS dmg image to deployment location
cp "$TEMPPATH"/naev-macos/*.dmg "$OUTDIR"/macos/naev-"$VERSION"-macos-universal.dmg

# Move Windows installer to deployment location
cp "$TEMPPATH"/naev-win64/naev*.exe "$OUTDIR"/win64/naev-"$VERSION"-win64.exe

# Move Dist to deployment location
cp "$TEMPPATH"/naev-dist/source.tar.xz "$OUTDIR"/dist/naev-"$VERSION"-source.tar.xz

# Move Soundtrack to deployment location if this is a release.
if [ "$NIGHTLY" == "true" ] || [ "$PRERELEASE" == "true" ]; then
   echo "not preparing soundtrack"
else
   cp "$TEMPPATH"/naev-soundtrack/naev-*-soundtrack.zip "$OUTDIR"/dist/naev-"$VERSION"-soundtrack.zip
fi

# Print staged files for verification
ls -l -R "$OUTDIR"
