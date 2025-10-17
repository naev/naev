#!/bin/bash

readarray -t FILES <<< "$(git ls-files "*/nlua_*.c")"
./utils/luacheckrc_extractor.py "${FILES[@]}" --output utils/luacheckrc_gen.lua
