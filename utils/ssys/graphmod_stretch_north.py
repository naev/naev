#!/usr/bin/env python3


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import stderr, argv, exit

if argv[1:]:
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + '\n'
      '  Reads a graph file on stdin, outputs a graph on stdout.\n'
      '  Horizontally stretches the north.\n'
   )
   exit(0)

from geometry import vec, bb
from graphmod import ssys_pos as pos

box = bb()
for (_, v) in pos.items():
   box += v

STRETCH = 0.1
def f(x):
   x = 1.0 - x
   if x > 0.5:
      return 1.0
   elif x < 0.25:
      return 1.0 + STRETCH
   else:
      x = x*4.0 - 1.0
      return 1.0 + STRETCH * (1.0 - x)

anbh = { 'ngc11935', 'ngc5483', 'ngc7078', 'ngc7533', 'octavian',
   'copernicus', 'ngc13674', 'ngc1562', 'ngc2601', 'zied'}
for (k, v) in pos.items():
   if k not in anbh:
      t= (v[1] - box.miny) / (box.maxy - box.miny)
      pos[k] = vec(pos[k][0] * f(t), pos[k][1])
