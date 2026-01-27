#!/usr/bin/env bash

# an internal component of process_ssys (used repeatedly there)
SCRIPT_DIR="$1"
shift
"$SCRIPT_DIR"/graphmod_smooth.py              |
"$SCRIPT_DIR"/det_reposition.sh -e -q -i "$@" | {
   cat
   echo -e -n "\e[32m.\e[0m" >&2
}
