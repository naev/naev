#!/usr/bin/bash

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage:  $(basename "$0")" >&2
   echo "  Output the ssys graph (positions and edges)." >&2
   echo "  See ssys_pos.sh and ssys.edges.sh." >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
"$SCRIPT_DIR"/ssys_pos.sh
"$SCRIPT_DIR"/ssys_edges.sh
