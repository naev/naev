#!/bin/bash
[ -x "$(command -v luacheck)" ] && exec luacheck "$@" || exit 0
