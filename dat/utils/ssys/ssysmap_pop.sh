#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../ssys")" &&

grep -l '<opos x="' ./*.xml | while read -r i ; do
   sed -e '0,/<pos x="/{//d}' -e '0,/<opos x="/s//<pos x="/' -i "$i"
done
