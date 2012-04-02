#!/usr/bin/env bash

cd "$(dirname $0)"

export LC_ALL=C
data="../../dat/outfits"

# XML elements which use the directories.
store="gfx_store"
space="gfx"

echo "Checking for unused graphics..."
for dir in store space; do
   cd "$dir"
   echo -e "\n   Unused outfit $dir gfx"
   for img in *.png; do
      [[ $img =~ -end.png ]] && continue
      if ! cat ../${data}/*/*.xml | grep -q "<${!dir}[^_]*>${img%.png}<"; then
         echo "      $img"
      fi
   done
   cd ..
done

echo -e "\nChecking for overused graphics..."
for dir in store space; do
   cd "$dir"
   echo -e "\n   Overused outfit $dir gfx"
   for img in *.png; do
      [[ $img =~ -end.png ]] && continue
      count=$(cat ../${data}/*/*.xml | grep -c "<${!dir}[^_]*>${img%.png}<")
      if [[ $count > 1 ]]; then
         echo "      $img => $count times"
      fi
   done
   cd ..
done
