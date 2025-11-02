#!/bin/bash

readarray -t FILES <<< "$(git ls-files "src/nlua_*.c" "src/ai.c" )"
TMPFILE=$(mktemp)
./utils/luacheckrc_extractor.py "${FILES[@]}" --output "$TMPFILE"
cmp --silent "$TMPFILE" utils/luacheckrc_gen.lua || cp "$TMPFILE" utils/luacheckrc_gen.lua
rm "$TMPFILE"
