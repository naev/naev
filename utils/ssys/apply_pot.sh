#!/usr/bin/bash

N=10
RESCALE=1.7

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage:  $(basename "$0") -C | ( -g | -w )" >&2
   echo "  Applies potential $N times and rescales x$RESCALE." >&2
   echo "  -g stands for gravity; -w for waves." >&2
   echo "  See potential -h for more information." >&2
   echo "  Output the positions of systems in the same form as ssys_pos.sh." >&2
   echo "  If -C is set, just compile potential." >&2
   exit 0
fi

repiper() {
   local -i n="$1";
   n="$((n-1))"
   shift;
   if (( n )); then
      "$@" | repiper "$n" "$@"
   else
      "$@"
   fi
}

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ ! -f "$SCRIPT_DIR"/potential ] || [ ! "$SCRIPT_DIR"/potential -nt "$SCRIPT_DIR"/potential.c ] ; then
   gcc -Wall -Wextra -Ofast "$SCRIPT_DIR"/potential.c -o "$SCRIPT_DIR"/potential -lm || exit 1
fi

for j in "$@"; do
   if [ "$j" = "-C" ]; then
      exit 0
   fi
done

"$SCRIPT_DIR"/ssys_pos.sh |
repiper "$N" "$SCRIPT_DIR"/potential -a "$1" |
"$SCRIPT_DIR"/ssys_graph.py -s "$RESCALE"
