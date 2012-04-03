#!/usr/bin/env bash

cd "$(dirname $0)"

export LC_ALL=C
data="../../dat/assets"

echo "Checking for unused graphics..."
for dir in space exterior; do
   cd "$dir"
   echo -e "\n   Unused planet $dir gfx"
   for img in *.png; do
      if ! cat ../${data}/*.xml | grep -qF "<$dir>$img"; then
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
      count=$(cat ../${data}/*.xml | grep -cF "<$dir>$img")
      if [[ $count > 1 ]]; then
         echo "      $img => $count times"
      fi
   done
   cd ..
done
