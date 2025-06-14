#!/usr/bin/bash

N=10
RESCALE=1.7

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

"$SCRIPT_DIR"/ssys_pos.sh |
repiper "$N" "$SCRIPT_DIR"/gravity -a |
"$SCRIPT_DIR"/ssys_graph.py -s "$RESCALE"
