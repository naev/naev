#!/usr/bin/env python3


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import stderr, argv, exit

if argv[1:] != []:
   stderr.write('usage: ' + argv[0].split('/')[-1] + '\n')
   stderr.write('  Reads a graph file on stdin, outputs a graph on stdout.\n')
   stderr.write('  Intended as a postprocessing for neato output.\n')
   exit(0)

from geometry import vec
from graphmod import sys_pos as pos


# Smoothen tradelane
from ssys_graph import xml_files_to_graph
# TODO: get tradelane directly from input
_V, _pos, E, tradelane, _color = xml_files_to_graph()

newp = dict()
for k in pos:
   if k[0] != '_' and (k in tradelane):
      tln = [s for (s, _) in E[k] if (s in tradelane)]
      if (n := len(tln)) > 1:
         p = sum([pos[s] for s in tln], vec())
         newp[k] = pos[k] * (1.0 - n*0.125)  +  p * 0.125

for k, v in newp.items():
   pos[k] = v
