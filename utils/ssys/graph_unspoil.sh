#!/usr/bin/bash


TMP=$(mktemp)
trap 'rm -f "$TMP"' EXIT

tee >(grep -v '\<hidden\>' > "$TMP")      |
grep '\(\<spoiler\>\)\|\(\<corridor\>\)'  |
sed 's/^\([^ ]*\>\).*/\\<\1\\>/'          |
grep -v -E -f - "$TMP"
