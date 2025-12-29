#!/usr/bin/env bash

cd "$1" || exit
if [ -d .git ]; then
   unset GIT_INDEX_FILE  # Don't let whatever meson does fuck up the results.
   git ls-files -- "./**.xml"
else
   find . -name "*.xml"
fi
