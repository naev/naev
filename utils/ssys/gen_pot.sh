#!/usr/bin/bash

gcc -Wall -Wextra -Ofast utils/ssys/graph2potential.c -o graph2potential -lm
./utils/ssys/ssys2graph.py | ./graph2potential
pnmtopng out.pgm > pot.png
rm out.pgm
ls -l pot.png >&2
