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

pos['fertile_crescent'] += 0.3 * (pos['fertile_crescent'] - pos['ngc4771'])
pos['ngc7061'] += 0.4 * (pos['kansas']-pos['sylph'])
