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

gcc -Wall -Wextra -Ofast utils/ssys/gravity.c -o gravity -lm &&
./utils/ssys/ssys_graph.py | repiper "$N" ./gravity -a | ./utils/ssys/ssys_graph.py -w "$SCALE"
