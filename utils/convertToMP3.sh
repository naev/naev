#!/usr/bin/env bash

set -e

usage() {
    echo "usage: $(basename "$0") [-d] -i <inputDir> (no trailing '/') -e <fileextension> -o <outputDir> (no trailing '/')"
    cat <<EOF
Renders audio to mp3, while retaining metadata
Requires ffmpeg to be installed and available on PATH

Pass in -i <inputDir> (no trailing '/') -f <fileextension> -o <outputDir> (no trailing '/')
EOF
    exit 1
}

while getopts d:f:i:o: OPTION "$@"; do
    case $OPTION in
    d)
        set -x
        ;;
    f)
        FILEEXTENSION="${OPTARG}"
        ;;
    i)
        INPUTDIR="${OPTARG}"
        ;;
    o)
        OUTPUTDIR="${OPTARG}"
        ;;
    *)
        usage
        ;;
    esac
done

if [ -z "$FILEEXTENSION" ] || [ -z "$OUTPUTDIR" ] || [ -z "$INPUTDIR" ]; then
    usage
fi

mkdir -p "$OUTPUTDIR"
for f in "$INPUTDIR"/*."$FILEEXTENSION"; do
    OUTFILE=$(basename "$f" ".$FILEEXTENSION")
    echo "Converting $f to $OUTFILE.mp3"
    ffmpeg -n -i "$f" -c:a libmp3lame -q:a 1 -ar 44100 -map_metadata 0 -map_metadata 0:s:0 -id3v2_version 3 -vn "$OUTPUTDIR/$OUTFILE.mp3"
done
