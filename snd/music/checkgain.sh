#!/usr/bin/env bash

for FILE in *.ogg; do
   if [ -z "$(vorbiscomment -l $FILE | grep REPLAYGAIN_TRACK)" ]; then
      echo $FILE;
   fi;
done
