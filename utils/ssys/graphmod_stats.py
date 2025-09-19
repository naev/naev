#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, stderr
from graph_vaux import color_values

if {'-h', '--help'} & set(argv[1:]):
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + '\n'
      '  Reads a graph file on stdin, outputs the same graph on stdout.\n'
      '  Writes some stats on stderr.\n'
   )
   exit(0)

from graphmod import ssys_pos
from geometry import vec

g = sum(ssys_pos.values(), vec()) / len(ssys_pos)
minx = min((x for x, _y  in ssys_pos.values()))
maxx = max((x for x, _y  in ssys_pos.values()))
miny = min((y for _x, y  in ssys_pos.values()))
maxy = max((y for _x, y  in ssys_pos.values()))

d = max(maxx-minx,maxy-miny)
f = 2.0/d
UL = vec(minx, miny)
DR = vec(maxx, maxy)
middle = 0.5 * (UL + DR)
rel_pos = lambda v: f*(v-middle)
def disp( v ):
   x,y=v
   return str((round(100*x), round(100*y)))

s= ' '.join(argv[1:])
stderr.write(s + '  window ' + disp(rel_pos(UL)) + '--' + disp(rel_pos(DR)) + '\n\n')
stderr.write(s + '  centre of mass' + disp(rel_pos(g)) + '\n')
for i in ['sol', 'gamma_polaris', 'feye', 'dvaer', 'zalek', 'aesir']:
   stderr.write(s + '  ' + i + ': ' + disp(rel_pos(ssys_pos[i])) + '\n')
