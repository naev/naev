#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import argv, stderr, exit
if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)>3:
   stderr.write('Usage: ' + argv[0] + '<x> <y>' + '\n')
   stderr.write('Read a graph in input, and output the vertex closest to <x> <y>\n' + '\n')
   exit(0)

from graphmod import sys_pos as pos, no_graph_out
from geometry import vec

no_graph_out()

v = vec(float(argv[1]), float(argv[2]))

best, which = None, None
for n, p in pos.items():
   dif = p - v
   crt = dif * dif
   if best is None or crt < best:
      best, which = crt, n

v -= pos[which]
print (which, '+',  v)
