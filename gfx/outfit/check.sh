#!/usr/bin/env bash

DATA="../../dat/outfit.xml"

echo "Checking for unused graphics..."
echo

# Check unused space gfx
echo "   Unused outfit store gfx"
cd store
for SPACE in *.png; do
   if [ -z "`grep ">${SPACE%.png}<" ../$DATA | grep "<gfx_store>"`" ]; then
      echo "      $SPACE"
   fi
done
cd ..

# Check unused exterior gfx
echo "   Unused outfit gfx"
cd space
for SPACE in *.png; do
   if [ -z "`grep ">${SPACE%.png}<" ../$DATA | grep "<gfx>"`" ]; then
      echo "      $SPACE"
   fi
done
cd ..

echo
echo
echo "Checking for overused graphics..."
echo

# Check overused
echo "   Overused outfit store gfx"
cd store
for SPACE in *.png; do
   COUNT=`grep ">${SPACE%.png}<" ../$DATA | grep -c "<gfx_store>"`
   if [ $COUNT -gt 1 ]; then
      echo "      $SPACE => $COUNT times"
   fi
done
cd ..

# Check unused exterior gfx
echo "   Overused outfit gfx"
cd space
for SPACE in *.png; do
   COUNT=`grep ">${SPACE%.png}<" ../$DATA | grep -c "<gfx>"`
   if [ $COUNT -gt 1 ]; then
      echo "      $SPACE => $COUNT times"
   fi
done
cd ..
