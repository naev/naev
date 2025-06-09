#!/usr/bin/bash

gcc -Wall -Wextra -Ofast utils/ssys/gravity.c -o gravity -lm &&
./utils/ssys/ssys_graph.py | ./gravity &&
pnmtopng out.pgm > pot.png ;
mv out.pgm pot.pgm ;
ls -l pot.pgm pot.png >&2 ;
