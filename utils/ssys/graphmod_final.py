#!/usr/bin/env python3


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import stderr, argv, exit

if argv[1:] != []:
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + '\n'
      '  Reads a graph file on stdin, outputs a graph on stdout.\n'
      '  Post-gravity final corrections.\n'
   )
   exit(0)

from graphmod import sys_pos as pos
from geometry import segment, circumscribed


# Repair collective area symetry

C = circumscribed(pos['c59'], pos['c43'], pos['c28'])

S1 = segment(pos['c59'], pos['c43'])
Pts1 = S1.bisector() & C
if (Pts1[0]-pos['c59']).size() < (Pts1[-1]-pos['c59']).size():
   Pt1 = Pts1[-1]
else:
   Pt1 = Pts1[0]

S2 = segment(pos['c59'], pos['c28'])
Pts2 = S2.bisector() & C
if (Pts2[0]-pos['c59']).size() < (Pts2[-1]-pos['c59']).size():
   Pt2 = Pts2[-1]
else:
   Pt2 = Pts2[0]

pos['c00'] = Pt1
pos['c14'] = Pt2


# Twist Carnis M{in,aj}or

C = (pos['carnis_minor'] + pos['carnis_major']) / 2.0
pos['carnis_minor'] = C + (pos['carnis_minor'] - C).rotate(110)
pos['carnis_major'] = C + (pos['carnis_major'] - C).rotate(110)
d = pos['carnis_major'] - pos['carnis_minor']
pos['carnis_minor'] += d / 2.0
pos['carnis_major'] += d / 2.0


# ABH *again*

L = segment(pos['ngc2601'], pos['ngc1562']).bisector()
have = L.v.normalize()
want = (pos['anubis_black_hole'] - L.P).normalize()
pos['ngc2601'] = pos['ngc1562'] + (pos['ngc2601'] - pos['ngc1562']) * (want/have)

L = segment(pos['ngc5483'], pos['ngc11935']).bisector()
have = L.v.normalize()
want = (pos['anubis_black_hole'] - L.P).normalize()
pos['ngc11935'] = pos['ngc5483'] + (pos['ngc11935'] - pos['ngc5483']) * (want/have)
