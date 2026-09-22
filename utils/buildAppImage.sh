#!/usr/bin/env bash

# AppDir BUILD SCRIPT FOR NAEV
#
# For more information, see http://appimage.org/
# Pass in [-d] (set this for debug builds) [-n] (set this for nightly builds) [-i] (set this to build an appimage from source) [-p] (set this to package an appimage from an AppDir) [-e] (set this when the AppDir already holds a meson install, skipping the build) -a <APPDIRPATH> Sets location of AppDir for packaging -s <SOURCEPATH> (Sets location of source) -b <BUILDPATH> (Sets location of build directory) [-c] (set this to skip slow compression)

# Output destination is ${WORKPATH}/dist

set -e
set -o pipefail

# Defaults
SOURCEPATH="$(pwd)"
BUILDTYPE="debug"
MAKEAPPIMAGE="false"
PACKAGE="false"
NIGHTLY="false"
PREBUILT="false"

# Parse arguments
while getopts dnipea:s:b:c OPTION "$@"; do
   case $OPTION in
   d)
      set -x
      BUILDTYPE="debug"
      ;;
   n)
      NIGHTLY="true"
      BUILDTYPE="debug"
      ;;
   i)
      MAKEAPPIMAGE="true"
      ;;
   p)
      PACKAGE="true"
      ;;
   e)
      PREBUILT="true"
      ;;
   a)
      APPDIRPATH="${OPTARG}"
      ;;
   s)
      SOURCEPATH="${OPTARG}"
      ;;
   b)
      BUILDPATH="${OPTARG}"
      ;;
   c)
      NO_COMPRESSION="true"
      ;;
   *)
      ;;
   esac
done

# Setup working paths
if [ -z "$BUILDPATH" ]; then
   BUILDPATH="$(mktemp -d)"
   WORKPATH=$(readlink -mf "$BUILDPATH")
else
   WORKPATH=$(readlink -mf "$BUILDPATH")
fi

if [ -z "$APPDIRPATH" ]; then
   APPDIRPATH="$WORKPATH/dist/AppDir"
else
   APPDIRPATH=$(readlink -mf "$APPDIRPATH")
fi

BUILDPATH="$WORKPATH/builddir"

# Output configured variables
echo "SCRIPT WORKING PATH: $WORKPATH"
echo "APPDIR PATH:       $APPDIRPATH"
echo "SOURCE PATH:       $SOURCEPATH"
echo "BUILD PATH:        $BUILDPATH"
echo "BUILDTYPE:         $BUILDTYPE"

# Make temp directories
mkdir -p "$WORKPATH"/{dist,utils}

# Get arch for use with linuxdeploy and to help make the linuxdeploy URL more architecture agnostic.
#ARCH=$(arch)
ARCH=$(uname -m)
export ARCH

get_tools() {
   # Get linuxdeploy's AppImage
   linuxdeploy="$WORKPATH/utils/linuxdeploy.AppImage"
   if [ ! -f "$linuxdeploy" ]; then
      curl -L -o "$linuxdeploy" "https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20250213-2/linuxdeploy-$ARCH.AppImage" \
         || { echo "Failed to download linuxdeploy"; exit 1; }
      #
      # This fiddles with some magic bytes in the ELF header. Don't ask me what this means.
      # For the layman: makes appimages run in docker containers properly again.
      # https://github.com/AppImage/AppImageKit/issues/828
      #
      sed '0,/AI\x02/{s|AI\x02|\x00\x00\x00|}' -i "$linuxdeploy"
      chmod +x "$linuxdeploy"
   fi
   # Get appimagetool's AppImage
   appimagetool="$WORKPATH/utils/appimagetool.AppImage"
   if [ ! -f "$appimagetool" ]; then
      curl -L -o "$appimagetool" "https://github.com/AppImage/appimagetool/releases/download/1.9.0/appimagetool-$ARCH.AppImage" \
         || { echo "Failed to download appimagetool"; exit 1; }
      #
      # This fiddles with some magic bytes in the ELF header. Don't ask me what this means.
      # For the layman: makes appimages run in docker containers properly again.
      # https://github.com/AppImage/AppImageKit/issues/828
      #
      sed '0,/AI\x02/{s|AI\x02|\x00\x00\x00|}' -i "$appimagetool"
      chmod +x "$appimagetool"
   fi
}

build_appdir() {
   if [[ "$PREBUILT" != "true" ]]; then
      PROFILE=()
      if [ "$BUILDTYPE" = "release" ]; then
         PROFILE=(--release)
      fi
      # Keep cargo's artifacts under the build path the caller asked for.
      export CARGO_TARGET_DIR="$BUILDPATH"
      CARGO_ARGS=(--manifest-path "$SOURCEPATH/Cargo.toml")

      # install builds the engine with the prefix it is about to install to,
      # so the compiled-in data path and the staged data agree. Steam's runtime
      # lacks the numeric libraries, hence linking them in.
      DESTDIR="$APPDIRPATH" cargo run --quiet "${CARGO_ARGS[@]}" --package xtask --release -- \
         install --prefix /usr --features steamruntime "${PROFILE[@]}"
   fi
   # Rename metainfo file
   mv "$APPDIRPATH/usr/share/metainfo/org.naev.Naev.metainfo.xml" "$APPDIRPATH/usr/share/metainfo/org.naev.Naev.appdata.xml"
   pushd "$WORKPATH"
   "$linuxdeploy" --appdir "$APPDIRPATH"
   popd
}

build_appimage() {
   # Set VERSION and OUTPUT variables
   if [ -f "$APPDIRPATH/usr/share/naev/dat/VERSION" ]; then
      VERSION="$(<"$APPDIRPATH/usr/share/naev/dat/VERSION")"
      export VERSION
   else
      echo "The VERSION file is missing from $APPDIRPATH."
      exit 1
   fi

   if [[ "$NIGHTLY" =~ "true" ]]; then
      TAG="nightly"
   else
      TAG="latest"
   fi

   SUFFIX="$VERSION-linux"

   if [[ "$ARCH" =~ "x86_64" ]]; then
      SUFFIX="$SUFFIX-x86-64"
   elif [[ "$ARCH" =~ "x86" ]]; then
      SUFFIX="$SUFFIX-x86"
   else
      SUFFIX="$SUFFIX-unknown"
   fi

   OUTPUT="$WORKPATH/dist/naev-$SUFFIX.AppImage"
   UPDATE_INFORMATION="zsync|https://codeberg.org/naev/naev/releases/download/$TAG/naev-*.AppImage.zsync"

   if [[ "$NO_COMPRESSION" =~ "true" ]]; then
      COMP_ARGS=(--mksquashfs-opt -no-compression)
   else
      COMP_ARGS=(--comp zstd --mksquashfs-opt -Xcompression-level --mksquashfs-opt 20)
   fi

   pushd "$WORKPATH/dist"
   "$appimagetool" "${COMP_ARGS[@]}" --no-appstream -v -u "$UPDATE_INFORMATION" "$APPDIRPATH" "$OUTPUT"
   popd
   echo "Completed."
}

get_tools

if [[ "$MAKEAPPIMAGE" =~ "true" ]]; then
   build_appdir
   build_appimage
elif [[ "$PACKAGE" =~ "true" ]]; then
   build_appimage
else
   build_appdir
fi
