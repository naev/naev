#!/usr/bin/env python3


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import stderr, argv, exit

if argv[1:] != []:
   stderr.write('usage: ' + argv[0].split('/')[-1] + '\n')
   stderr.write('  Reads a graph file on stdin, outputs a graph on stdout.\n')
   stderr.write('  Repositions Anubis Black Hole.\n')
   exit(0)

from geometry import vec, bounded_circles, bounding_circle
from graphmod import sys_pos as pos

anbh = [ 'ngc11935', 'ngc5483', 'ngc7078', 'ngc7533', 'octavian',
   'copernicus', 'ngc13674', 'ngc1562', 'ngc2601', 'zied']

def repos_center():
   for C in bounded_circles([pos[x] for x in anbh]):
      pos['anubis_black_hole'] = C.center
      break

F = 0.03
for k in range(2):
   repos_center()
   for i in anbh:
      pos[i] = vec( F*pos['anubis_black_hole'][0] + (1.0-F) * pos[i][0], pos[i][1])

C = bounding_circle([pos[x] for x in anbh])
pos['anubis_black_hole'] = C.center
