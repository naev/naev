#!/bin/bash
# Script to run luacheck on files changed since last commit.
# Usage: utils/luacheck_git.sh [--no-color]
cd "$(git rev-parse --show-toplevel)/dat"
# List all files that are modified/added/removed in the index, or modified in the working dir. Chop the "XY dat/" bit.
git status --porcelain '**.lua' | grep '^[MAR]\|^.M' | cut -b 8- | xargs luacheck "$@"
