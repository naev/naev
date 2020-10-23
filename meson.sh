#!/bin/sh

BUILDDIR="builddir"
MESONDIR="meson-bin"
MESON="$MESONDIR/meson.py"

VERSION="0.55.3"
PACKAGE="meson-$VERSION.tar.gz"

if [ ! -d "$MESONDIR" ]; then
    mkdir "$MESONDIR"
fi

if [ ! -f "$MESON" ]; then
    curl -L "https://github.com/mesonbuild/meson/releases/download/$VERSION/$PACKAGE" -o $PACKAGE
    tar -xf $PACKAGE -C "$MESONDIR" --strip 1
    rm $PACKAGE
fi

./meson-bin/meson.py $@