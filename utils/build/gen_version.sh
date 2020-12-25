#!/bin/sh

SOURCE_ROOT=$(realpath -m "$MESON_SOURCE_ROOT")

if [ -d "$SOURCE_ROOT/.git/" ]; then
    # In the git repo. Build the tag from git info
    VERSION=$(git -C "$SOURCE_ROOT" describe --tags --match 'v*' --dirty | sed -E 's/^v//;s/^(.*)-([^-]*)-(g[^-]*)/\1+\2.\3/;s/-dirty/.dirty/')
    echo $VERSION > "$SOURCE_ROOT/dat/VERSION"
    echo $VERSION

elif [ -f "$SOURCE_ROOT/dat/VERSION" ]; then
    # In a source package. Version file should exist.
    cat "$SOURCE_ROOT/dat/VERSION"

else
    # Couldn't find git repo or a packaged VERSION file
    # Did you download the zip'd repo instead of a release?
    VERSION="$1+dev"
    echo $VERSION > "$SOURCE_ROOT/dat/VERSION"
    echo $VERSION

fi
