#!/usr/bin/bash

N=10
RESCALE=1.7

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
   echo "usage:  $(basename "$0")" >&2
   echo "  Applies gravity $N times and rescales x$RESCALE." >&2
   echo "  Output the positions of systems in the same form as ssys_pos.sh." >&2
   echo "  If -C is set, just compile gravity." >&2
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

if [ ! -f "$SCRIPT_DIR"/gravity ] || [ ! "$SCRIPT_DIR"/gravity -nt "$SCRIPT_DIR"/gravity.c ] ; then
   gcc -Wall -Wextra -Ofast "$SCRIPT_DIR"/gravity.c -o "$SCRIPT_DIR"/gravity -lm || exit 1
fi

for j in "$@"; do
   if [ "$j" = "-C" ]; then
      exit 0
   fi
done

"$SCRIPT_DIR"/ssys_pos.sh |
repiper "$N" "$SCRIPT_DIR"/gravity -a |
"$SCRIPT_DIR"/ssys_graph.py -s "$RESCALE"
