#!/bin/bash

# AppDir BUILD SCRIPT FOR NAEV
#
# For more information, see http://appimage.org/
# Pass in [-d] (set this for debug builds) [-n] (set this for nightly builds) [-i] (set this to build an appimage from source) [-p] (set this to package an appimage from an AppDir) -a <APPDIRPATH> Sets location of AppDir for packaging -s <SOURCEPATH> (Sets location of source) -b <BUILDPATH> (Sets location of build directory)

# Output destination is ${WORKPATH}/dist

set -e

# Defaults
SOURCEPATH="$(pwd)"
BUILDTYPE="release"
MAKEAPPIMAGE="false"
PACKAGE="false"

while getopts dnipa:s:b: OPTION "$@"; do
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
    a)
        APPDIRPATH="${OPTARG}"
        ;;
    s)
        SOURCEPATH="${OPTARG}"
        ;;
    b)
        BUILDPATH="${OPTARG}"
        ;;
    *)
        ;;
    esac
done

# Creates temp dir if needed
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
echo "APPDIR PATH:         $APPDIRPATH"
echo "SOURCE PATH:         $SOURCEPATH"
echo "BUILD PATH:          $BUILDPATH"
echo "BUILDTYPE:           $BUILDTYPE"

# Make temp directories
mkdir -p "$WORKPATH"/{dist,utils}

# Get arch for use with linuxdeploy and to help make the linuxdeploy URL more architecture agnostic.
ARCH=$(arch)

export ARCH

get_tools(){
    # Get linuxdeploy's AppImage
    linuxdeploy="$WORKPATH/utils/linuxdeploy.AppImage"
    if [ ! -f "$linuxdeploy" ]; then
        curl -L -o "$linuxdeploy" "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-$ARCH.AppImage"
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
        curl -L -o "$appimagetool" "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-$ARCH.AppImage"
        #
        # This fiddles with some magic bytes in the ELF header. Don't ask me what this means.
        # For the layman: makes appimages run in docker containers properly again.
        # https://github.com/AppImage/AppImageKit/issues/828
        #
        sed '0,/AI\x02/{s|AI\x02|\x00\x00\x00|}' -i "$appimagetool"
        chmod +x "$appimagetool"
    fi
}

build_appdir(){
    # Honours the MESON variable set by the environment before setting it manually

    if [ -z "$MESON" ]; then
        MESON="$SOURCEPATH/meson.sh"
    fi

    "$MESON" setup "$BUILDPATH" "$SOURCEPATH" \
    --native-file "$SOURCEPATH/utils/build/linux_steamruntime_scout.ini" \
    --buildtype "$BUILDTYPE" \
    --force-fallback-for=glpk,SuiteSparse \
    -Dprefix="/usr" \
    -Db_lto=true \
    -Dauto_features=enabled \
    -Ddocs_c=disabled \
    -Ddocs_lua=disabled
    # Compile and Install Naev to DISTDIR
    DESTDIR=$APPDIRPATH "$MESON" install -C "$BUILDPATH"
    # Rename metainfo file
    mv "$APPDIRPATH/usr/share/metainfo/org.naev.Naev.metainfo.xml" "$APPDIRPATH/usr/share/metainfo/org.naev.Naev.appdata.xml"
    pushd "$WORKPATH"
    "$linuxdeploy" --appdir "$APPDIRPATH"
    popd

}

build_appimage(){
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

    export OUTPUT="$WORKPATH/dist/naev-$SUFFIX.AppImage"

    # Disable appstream test
    export NO_APPSTREAM=1

    export UPDATE_INFORMATION="gh-releases-zsync|naev|naev|$TAG|naev-*.AppImage.zsync"
    pushd "$WORKPATH/dist"
    "$appimagetool" -n -v -u "$UPDATE_INFORMATION" "$APPDIRPATH" "$OUTPUT"
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
