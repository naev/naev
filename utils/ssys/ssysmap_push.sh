#!/usr/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$(realpath --relative-to="$PWD" "${SCRIPT_DIR}/../../dat/ssys")" &&

for i in *.xml; do
   sed '0,/<pos x=/s/^\( *<\)pos\( x=".*\)$/\1pos\2\n\1opos\2/' -i "$i"
done
