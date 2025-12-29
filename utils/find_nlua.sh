#!/bin/bash

cd "$1" || exit
if [ -d .git ]; then
   unset GIT_INDEX_FILE  # Don't let whatever meson does fuck up the results.
   while read -r -a line; do
      LSFILES+=("${line[0]}")
   done < <(git ls-files "src/*.c" "src/**/*.c" "src/*.rs" "src/**/*.rs")
else
   while read -r -a line; do
      LSFILES+=("${line[0]}")
   done < <(find src/ -name "*.c" -or -name "*.rs")
fi

grep -l '@luamod' "${LSFILES[@]}"
