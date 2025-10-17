#!/bin/bash

readarray -t FILES <<< "$(git ls-files "src/nlua_*.c" "src/ai.c" )"
./utils/luacheckrc_extractor.py "${FILES[@]}" --output utils/luacheckrc_gen.lua
