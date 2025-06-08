#!/usr/bin/bash

gcc -Wall -Wextra -Ofast utils/ssys/gravity.c -o gravity -lm
./utils/ssys/ssys2graph.py |
./gravity -a |
./gravity -a |
./gravity -a |
./gravity -a |
./gravity -a |
./gravity -a |
./utils/ssys/ssys2graph.py -w 1.2
