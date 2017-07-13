#!/bin/bash
set -e

# This script migrates Naev's local files from deprecated locations to their
# new locations. Currently, this targets just macOS, moving files from XDG
# directories to macOS standard directories.

# Check for macOS.
[[ "$(uname)" == "Darwin" ]] || exit

# Set defaults if XDG variables aren't defined.
test -z "$XDG_DATA_HOME" && XDG_DATA_HOME=$HOME/.local/share
test -z "$XDG_CONFIG_HOME" && XDG_CONFIG_HOME=$HOME/.config
test -z "$XDG_CACHE_HOME" && XDG_CACHE_HOME=$HOME/.cache

# Determine Naev directories.
XDG_DATA="$XDG_DATA_HOME/naev"
XDG_CONFIG="$XDG_CONFIG_HOME/naev"
XDG_CACHE="$XDG_CACHE_HOME/naev"

# Determine Naev directories on macOS.
MAC_DATA="$HOME/Library/Application Support/org.naev.Naev"
MAC_CONFIG="$HOME/Library/Preferences/org.naev.Naev"
MAC_CACHE="$HOME/Library/Caches/org.naev.Naev"

# Attempt to migrate each of the XDG directories to macOS directories.
if compgen -G "$XDG_DATA/*" > /dev/null && mkdir -p $MAC_DATA; then
   mv -n "$XDG_DATA"/* "$MAC_DATA/"
fi
rmdir $XDG_DATA 2> /dev/null || true

if compgen -G "$XDG_CONFIG/*" > /dev/null && mkdir -p $MAC_CONFIG; then
   mv -n "$XDG_CONFIG"/* "$MAC_CONFIG/"
fi
rmdir $XDG_CONFIG 2> /dev/null || true

if compgen -G "$XDG_CACHE/*" > /dev/null && mkdir -p $MAC_CACHE; then
   mv -n "$XDG_CACHE"/* "$MAC_CACHE/"
fi
rmdir $XDG_CACHE 2> /dev/null || true
