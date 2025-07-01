#!/usr/bin/env python3


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import stderr, argv, exit

try:
   scale = float(argv[1])
except:
   scale = None

if scale is None:
   stderr.write('usage: ' + argv[0].split('/')[-1] + ' <scale>\n')
   stderr.write('  Reads a graph, scales it, outputs the result.\n')
   exit(0 if '-h' in argv[1:] else 1)

from graphmod import sys_pos as pos

for t in pos:
   pos[t] *= scale
