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

from graphmod import sys_pos, sys_jmp as E
from smoothen import smoothen


neigh = { k: {s for (s, t) in E[k] if 'tradelane' in t} for k in sys_pos }
for k, v in smoothen(sys_pos, neigh).items():
   sys_pos[k] = v
