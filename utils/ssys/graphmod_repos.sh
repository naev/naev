#!/usr/bin/bash

# an internal component of process_ssys (used repeatedly there)
SCRIPT_DIR="$1"
shift
"$SCRIPT_DIR"/graphmod_smooth_tl.py -e |
"$SCRIPT_DIR"/reposition -e -q -i "$@" |
(
   cat
   echo -e -n "\e[32m.\e[0m" >&2
)
