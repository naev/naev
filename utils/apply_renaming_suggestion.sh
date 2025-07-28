#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../")

tmp=$(mktemp)
trap 'rm -f $tmp' EXIT

sed 's/^\(.*\){\(.*\) => \(.*\)\}.xml$/\1 \2.xml \3.xml/' |
while read -r line; do
   path=$(echo "$line" | cut '-d ' -f 1)
   from=$(echo "$line" | cut '-d ' -f 2)
   to=$(echo "$line" | cut '-d ' -f 3)
   git mv "$path$from" "$path$to" &&
   echo "s/\/$from/\/$to/" >> "$tmp" &&
   echo "/$from"
done | grep -rlf - "$DST"/po "$DST"/dat 2>/dev/null | while read -r i; do
   echo "$i" >&2
   sed -f "$tmp" -i "$i"
done
