#!/usr/bin/env python3


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import stderr, argv, exit

if argv[1:] != []:
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + '\n'
      '  Reads a graph file on stdin, outputs a graph on stdout.\n'
      '  Intended as a postprocessing for neato output.\n'
   )
   exit(0)

from graphmod import ssys_pos, ssys_jmp
from smoothen import smoothen


neigh = { k: {s for (s, t) in ssys_jmp[k] if 'tradelane' in t} for k in ssys_pos }
for k, v in smoothen(ssys_pos, neigh).items():
   ssys_pos[k] = v
