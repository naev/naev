#!/usr/bin/env bash

PNGCRUSH=pngcrush

FILE="$1"
TMPFILE="/tmp/tmpfile.npngcrush"
SX="$2"
SY="$3"

# Safety checks
if [ -z $FILE ] || [ $SX -le 0 ] || [ $SY -le 0 ]; then
   echo "Usage is: $0 image sx sy"
   exit 1
fi

# Process
$PNGCRUSH -force -text b "sy" "$SY" $FILE $TMPFILE > /dev/null
$PNGCRUSH -force -text b "sx" "$SX" $TMPFILE $FILE > /dev/null

# Results
if [ -n "`identify -verbose $TMPFILE | grep sy`" ]; then
   echo "Success - $1 [$2x$3]"
else
   echo "Failed - $1 [$2x$3]"
fi


