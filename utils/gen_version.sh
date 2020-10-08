#!/bin/sh

cd "$MESON_SOURCE_ROOT"
if [[ -d .git ]]; then
    # In the git repo. Build the tag from git describe
    VERSION=$(git describe --tags --match 'v*' --dirty | sed -E 's/v(.*)-([^-]*)-(g[^-]*)/\1+\2.\3/;s/-dirty/.dirty/')
    echo $VERSION > "$MESON_SOURCE_ROOT/dat/VERSION"
    echo $VERSION
else
    # In a source package. Version file should exist.
    cat "$MESON_SOURCE_ROOT/dat/VERSION"
fi
