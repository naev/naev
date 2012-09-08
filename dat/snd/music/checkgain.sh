#!/usr/bin/env bash

if [ "$1" == "-h" ]; then
   echo "Run vorbisgain on the non-replaygain files to enable replaygain."
fi

for FILE in *.ogg; do
   if [[ -z $(vorbiscomment -l "$FILE" | grep REPLAYGAIN_TRACK) ]]; then
      echo $FILE;
   fi;
done
