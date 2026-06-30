#!/usr/bin/env python3


from sys import stderr, argv, exit


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

if argv[1:]:
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + '\n'
      '  Reads a graph file on stdin, outputs a graph on stdout.\n'
   )
   exit(0)


from geometry import bb, vec, segment, symmetry
from graphmod import ssys_pos as pos, ssys_jmp as E

box = bb()
for _k, v in pos.items():
   box += v

box_center = (box.mini() + box.maxi()) / 2.0
sol_center = pos['sol']
center = sol_center

stderr.write('center: '+str(center)+'\n')

L = list(pos.items())

for (k, v) in L:
   pos[k] -= center
