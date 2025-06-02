#!/usr/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST=$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../")

before=$(basename "$1")
after=$(basename "$2")
echo "$before => $after" >&2
#git mv -i "$1" "$2" &&
for i in $(grep -rl "$before" $DST/po $DST/dat 2>/dev/null) ; do
   echo $i
   sed "s/$before/$after/" -i "$i"
done
