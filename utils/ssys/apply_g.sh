#!/usr/bin/bash

N=8
SCALE=1.3

repiper() {
   local -i n="$1";
   n="$((n-1))"
   shift;
   if (( n )); then
      eval "$@" | repiper "$n" "$@"
   else
      eval "$@"
   fi
}

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
gcc -Wall -Wextra -Ofast "$SCRIPT_DIR"/gravity.c -o "$SCRIPT_DIR"/gravity -lm &&
"$SCRIPT_DIR"/ssys_graph.py | repiper "$N" "$SCRIPT_DIR"/gravity -a | "$SCRIPT_DIR"ssys_graph.py -w "$SCALE"
