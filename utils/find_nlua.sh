#!/bin/bash

cd "$1" || exit
if [ -d .git ]; then
   unset GIT_INDEX_FILE  # Don't let whatever meson does fuck up the results.
   readarray -t LSFILES <<< "$(git ls-files "src/*.c" "src/**/*.c" "src/*.rs" "src/**/*.rs")"
else
   readarray -t LSFILES <<< "$(find src/ -name "*.c" -or -name "*.rs")"
fi
grep -l '@luamod' "${LSFILES[@]}"
