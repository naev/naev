#!/bin/bash
luacheck "$@"
[ $? -ge 1 ] && exit 0 || exit 1
