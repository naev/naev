#!/bin/bash

#readarray -t FILES <<< "$(git ls-files "src/nlua_*.c" "src/ai.c" )"
#readarray -t LSFILES <<< "$(git ls-files "src/**/*.c" "src/**/*.rs")"
#readarray -t LSFILES <<< "$(git ls-files "src/*.c" "src/**/*.c" "src/*.rs" "src/**/*.rs")"
#readarray -t FILES <<< "$(grep -l '@luamod' "${LSFILES[@]}")"
readarray -t FILES <<< "$(./utils/find_nlua.sh .)"
TMPFILE=$(mktemp)

#./utils/luacheckrc_extractor.py "${FILES[@]}" --output "$TMPFILE"
cargo run --manifest-path utils/docmaker/Cargo.toml luacheck "${FILES[@]}" --output "$TMPFILE"

cmp --silent "$TMPFILE" utils/luacheckrc_gen.lua || cp "$TMPFILE" utils/luacheckrc_gen.lua
rm "$TMPFILE"
