#!/usr/bin/bash

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   DOC=(
      "usage:  $(basename "$0")  ( -g | -w )"
      "  Generates a (downscaled) map \"pot.png\" of the potential given in argument."
      "  -g stands for gravity; -w for waves."
      "  See potential -h for more information."
      "  povray is invoked to generate \"height_map.png\"."
   )
   ( IFS=$'\n'; echo "${DOC[*]}" ) >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
if [ ! -f "$SCRIPT_DIR"/potential ] || [ ! "$SCRIPT_DIR"/potential -nt "$SCRIPT_DIR"/potential.c ] ; then
   gcc -Wall -Wextra -Ofast "$SCRIPT_DIR"/potential.c -o "$SCRIPT_DIR"/potential -lm || exit 1
fi
"$SCRIPT_DIR"/ssys2graph.sh -v | "$SCRIPT_DIR"/potential -s0.5 "$1" | pnmtopng > "$SCRIPT_DIR"/pot.png &&
BACK=$(pwd) &&
cd "$SCRIPT_DIR" &&
povray +A0.1 +AM2 +R3 "$SCRIPT_DIR"/height_map.pov +O"$SCRIPT_DIR"/height_map.png &&
cd "$BACK" &&
ls -l "$SCRIPT_DIR"/pot.png "$SCRIPT_DIR"/height_map.png >&2
