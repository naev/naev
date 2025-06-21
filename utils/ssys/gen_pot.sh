#!/usr/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
if [ ! -f "$SCRIPT_DIR"/gravity ] || [ ! "$SCRIPT_DIR"/gravity -nt "$SCRIPT_DIR"/gravity.c ] ; then
   gcc -Wall -Wextra -Ofast "$SCRIPT_DIR"/gravity.c -o "$SCRIPT_DIR"/gravity -lm || exit 1
fi
"$SCRIPT_DIR"/ssys_graph.py | "$SCRIPT_DIR"/gravity -s 0.5 | pnmtopng > "$SCRIPT_DIR"/pot.png &&
BACK=$(pwd) &&
cd "$SCRIPT_DIR" &&
povray +A0.1 +AM2 +R3 "$SCRIPT_DIR"/height_map.pov +O"$SCRIPT_DIR"/height_map.png &&
cd "$BACK" &&
ls -l "$SCRIPT_DIR"/pot.png "$SCRIPT_DIR"/height_map.png >&2
