#!/usr/bin/env python3


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import stderr, argv, exit

if argv[1:] != []:
   stderr.write('usage: ' + argv[0].split('/')[-1] + '\n')
   stderr.write('  Reads a graph file on stdin, outputs a graph on stdout.\n')
   stderr.write('  Horizontally stretches the north.\n')
   exit(0)

from geometry import vec, bb
from graphmod import sys_pos as pos

box = bb()
for (_, v) in pos.items():
   box += v

STRETCH = 0.20
def f(x):
   x = 1.0 - x
   x = 2.0*x
   if x > 1.0:
      return 1.0
   else:
      return 1.0 + STRETCH * (1.0 - x)

for (k, v) in pos.items():
   t= (v[1] - box.miny) / (box.maxy - box.miny)
   pos[k] = vec(pos[k][0] * f(t), pos[k][1] / f(t))
