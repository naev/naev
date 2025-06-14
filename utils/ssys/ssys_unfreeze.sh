#!/usr/bin/bash

if [ "$#" = "0" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage:  $(basename "$0")  <file1> .." >&2
   echo "  Relaxes and unfreezes its input xml ssys files." >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
"$SCRIPT_DIR/ssys_relax.py" "$@"
sed -i 's/<pos x=\".*\" was_auto=\"true\"\/>/<autopos\/>/' "$@"
