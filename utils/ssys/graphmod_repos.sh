#!/usr/bin/bash

SCRIPT_DIR="$1"
shift
"$SCRIPT_DIR"/reposition -e -q -i "$@" |
"$SCRIPT_DIR"/graphmod_smooth_tl.py |
(
   cat
   echo -e -n "\e[32m.\e[0m" >&2
)
