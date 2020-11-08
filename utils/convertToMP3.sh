#!/bin/bash
# Renders audio to mp3, while retaining metadata

# Pass in -i <inputDir> (no trailing '/') -f <fileextension> -o <outputDir> (no trailing '/')

set -e

while getopts d:f:i:o: OPTION "$@"; do
    case $OPTION in
    d)
        set -x
        ;;
    f)
        FILEEXTENSION=${OPTARG}
        ;;
    i)
        INPUTDIR=${OPTARG}
        ;;
    o)
        OUTPUTDIR=${OPTARG}
        ;;
    esac
done

if [[ -z "$FILEEXTENSION" || -z "$OUTPUTDIR" || -z "$INPUTDIR" ]]; then
    echo "usage: `basename $0` [-d] -i <inputDir> (no trailing '/') -e <fileextension> -o <outputDir> (no trailing '/')"
    exit 1
fi

mkdir -p $OUTPUTDIR
for f in $INPUTDIR/*.$FILEEXTENSION; do
    OUTFILE=$(basename "$f" .$FILEEXTENSION)
    echo "Converting $f to $OUTFILE.mp3"
    ffmpeg -n -i "$f" -c:a libmp3lame -q:a 1 -ar 44100 -map_metadata 0 -map_metadata 0:s:0 -id3v2_version 3 -vn $OUTPUTDIR/$OUTFILE.mp3 
done
