#!/usr/bin/env bash

cd $(dirname $0)

DATA="../../dat/asset.xml"

echo "Checking for unused graphics..."
echo

# Check unused space gfx
echo "   Unused planet space gfx"
cd space
for SPACE in *.png; do
   if [ -z "`grep $SPACE ../$DATA`" ]; then
      echo "      $SPACE"
   fi
done
cd ..

# Check unused exterior gfx
echo "   Unused planet exterior gfx"
cd exterior
for SPACE in *.png; do
   if [ -z "`grep $SPACE ../$DATA`" ]; then
      echo "      $SPACE"
   fi
done
cd ..

echo
echo
echo "Checking for overused graphics..."
echo

# Check overused
echo "   Overused planet space gfx"
cd space
for SPACE in *.png; do
   COUNT=`grep -c $SPACE ../$DATA`
   if [ $COUNT -gt 1 ]; then
      echo "      $SPACE => $COUNT times"
   fi
done
cd ..

# Check unused exterior gfx
echo "   Overused planet exterior gfx"
cd exterior
for SPACE in *.png; do
   COUNT=`grep -c $SPACE ../$DATA`
   if [ $COUNT -gt 1 ]; then
      echo "      $SPACE => $COUNT times"
   fi
done
cd ..
