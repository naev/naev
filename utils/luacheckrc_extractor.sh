#!/usr/bin/bash
python utils/luacheckrc_extractor.py $(git ls-files | grep "nlua_.*c$") --output utils/luacheckrc_gen.lua
