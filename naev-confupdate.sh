#!/bin/bash
# This script migrates Naev's data from the deprecated ~/.naev directory
# into a number of XDG-compliant directories. It aims to avoid data loss
# by only migrating explicitly Naev-owned files.

# Set defaults if XDG variables aren't defined.
[[ -z $XDG_DATA_HOME ]] && XDG_DATA_HOME=$HOME/.local/share
[[ -z $XDG_CONFIG_HOME ]] && XDG_CONFIG_HOME=$HOME/.config
[[ -z $XDG_CACHE_HOME ]] && XDG_CACHE_HOME=$HOME/.cache

# ~/.naev must exist.
[[ -d $HOME/.naev ]] || exit

# Handle weird cases where ~/.naev isn't readable/executable.
cd "$HOME/.naev" || exit

if mkdir -p "$XDG_DATA_HOME/naev"; then
   # Attempt to migrate each save, but don't overwrite existing ones.
   if [[ -d saves/ ]] && mkdir -p "$XDG_DATA_HOME/naev/saves"; then
      mv -n saves/*.ns "$XDG_DATA_HOME/naev/saves/"
      stat *.ns.backup >/dev/null 2>&1 && mv -n saves/*.ns.backup "$XDG_DATA_HOME/naev/saves"
   fi

   # Screenshots are numbered from zero, so old screenshots cannot coexist with new ones.
   [[ -d screenshots/ ]] && mv -n screenshots/ "$XDG_DATA_HOME/naev/"
fi

# Naev writes the config on exit. Clobber if necessary.
if mkdir -p "$XDG_CONFIG_HOME/naev"; then
   [[ -r conf.lua ]] && mv conf.lua "$XDG_CONFIG_HOME/naev/"
   [[ -r conf.lua.backup ]] && mv conf.lua.backup "$XDG_CONFIG_HOME/naev/"
fi

# Nebula images are generated on first-run if absent; we'll clobber the new ones if necessary.
if [[ -d gen/ ]] && mkdir -p "$XDG_CACHE_HOME/naev/nebula"; then
   mv gen/nebu_bg_*.png "$XDG_CACHE_HOME/naev/nebula/"
fi

# Clean up if nothing remains.
for dir in saves screenshots gen; do
   [[ -d $dir ]] && rmdir --ignore-fail-on-non-empty "$dir/"
done

rmdir --ignore-fail-on-non-empty "$HOME/.naev/"
