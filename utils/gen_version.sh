#!/bin/sh

if [[ -d .git ]]; then
    # In the git repo. Build the tag from git describe
    VERSION=$(git describe --tags --match 'v*' --dirty | sed -E 's/v(.*)-([^-]*)-(g[^-]*)/\1+\2.\3/;s/-dirty/.dirty/')
    echo $VERSION > dat/VERSION
    echo $VERSION
else
    # In a source package. Version file should exist.
    echo dat/VERSION
fi
