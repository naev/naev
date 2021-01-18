#!/usr/bin/env bash

cd "$(dirname $0)"

export LC_ALL=C
data="../../outfits"

# XML elements which use the directories.
store="gfx_store"
space="gfx"

echo "Checking for unused graphics..."
for dir in store space; do
   cd "$dir"
   echo -e "\n   Unused outfit $dir gfx"
   for img in *.{png,webp}; do
      [[ $img =~ -end.(png|webp) ]] && continue
      if ! cat ../${data}/*/*.xml | grep -q "<${!dir}[^_]*>${img}<"; then
         echo "      $img"
      fi
   done
   cd ..
done

echo -e "\nChecking for overused graphics..."
for dir in store space; do
   cd "$dir"
   echo -e "\n   Overused outfit $dir gfx"
   for img in *.{png,webp}; do
      [[ $img =~ -end.(png|webp) ]] && continue
      count=$(cat ../${data}/*/*.xml | grep -c "<${!dir}[^_]*>${img}<")
      if [[ $count > 1 ]]; then
         echo "      $img => $count times"
      fi
   done | sort -k3 -n -r
   cd ..
done
