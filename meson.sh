#!/bin/sh

MESONDIR="meson-bin"

grab_meson () {
    VERSION="0.55.3"
    PACKAGE="meson-$VERSION.tar.gz"

    if [ ! -d "$MESONDIR" ]; then
        mkdir "$MESONDIR"
    fi

    if [ ! -f "$MESON" ]; then
        echo "grabbing from online"
        curl -L "https://github.com/mesonbuild/meson/releases/download/$VERSION/$PACKAGE" -o $PACKAGE
        tar -xf $PACKAGE -C "$MESONDIR" --strip 1
        rm $PACKAGE
    else
        echo "using cached version $VERSION"
    fi
}

if ! [ -x "$(command -v meson)" ]; then
    echo "You don't have Meson in PATH"
    MESON="$MESONDIR/meson.py"
    grab_meson
else
    currentver="$(meson --version)"
    requiredver="0.55.0"
    if [ "$(printf '%s\n' "$requiredver" "$currentver" | sort -V | head -n1)" = "$requiredver" ]; then 
            echo "Meson version is greater than or equal to ${requiredver}"
            MESON="meson"
    else
            echo "Meson version is lower than ${requiredver}"
            MESON="$MESONDIR/meson.py"
            grab_meson
    fi
fi

run_meson () {
    if [ "$MESON" = *"$MESONDIR"* ]; then
        ./$MESON $@
    else
        $MESON $@
    fi
}

run_meson $@
