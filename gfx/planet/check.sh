#!/usr/bin/env bash

cd "$(dirname $0)"

export LC_ALL=C
data="../../dat/asset.xml"

echo "Checking for unused graphics..."
for dir in space exterior; do
   cd "$dir"
   echo -e "\n   Unused planet $dir gfx"
   for img in *.png; do
      if ! grep -qF "<$dir>$img" "../$data"; then
         echo "      $img"
      fi
   done
   cd ..
done

echo -e "\nChecking for overused graphics..."
for dir in space exterior; do
   cd "$dir"
   echo -e "\n   Overused planet $dir gfx"
   for img in *.png; do
      count=$(grep -cF "<$dir>$img" "../$data")
      if [[ $count > 1 ]]; then
         echo "      $img => $count times"
      fi
   done
   cd ..
done
