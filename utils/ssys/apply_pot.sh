#!/usr/bin/bash

if [ "$1" = "-g" ] ; then
   N=20
   RESCALE=1.8
   POST_RESCALE=1.1
elif [ "$1" = "-E" ] ; then
   N=20
   RESCALE=0.95
   POST_RESCALE=1.5
else
   N=15
   RESCALE=1.0
   POST_RESCALE=1.0
fi

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage:  $(basename "$0") -C | ( -E | -g | -w )" >&2
   echo "  Applies potential $N times and rescales x$RESCALE to input graph." >&2
   echo "  -g stands for gravity; -w for waves. (-E for experimental grav.)" >&2
   echo "  See potential -h for more information." >&2
   echo "  Output the positions of systems in the same form as ssys2graph.sh." >&2
   echo "  If -C is set, just compile potential." >&2
   exit 0
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ ! -f "$SCRIPT_DIR"/potential ] || [ ! "$SCRIPT_DIR"/potential -nt "$SCRIPT_DIR"/potential.c ] ; then
   echo -n 'compile potential.. ' >&2
   gcc -Wall -Wextra -Ofast "$SCRIPT_DIR"/potential.c -o "$SCRIPT_DIR"/potential -lm || exit 1
   echo >&2
fi

for j in "$@"; do
   if [ "$j" = "-C" ]; then exit 0; fi
done

"$SCRIPT_DIR"/graph_scale.py "$RESCALE" |
"$SCRIPT_DIR"/repeat.sh "$N" "$SCRIPT_DIR"/potential -a "$1" |
"$SCRIPT_DIR"/graph_scale.py "$POST_RESCALE"
