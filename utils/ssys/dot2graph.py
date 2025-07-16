#!/usr/bin/env python3


from sys import stdin, stderr, argv, exit
from geometry import bb, vec
import os
import xml.etree.ElementTree as ET



if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

if argv[1:] != []:
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + '\n'
      '  Reads a dot file on stdin, writes the graph on stdout.\n'
   )
   exit(0)

def input_blocks( it ):
   acc = ''
   for line in it:
      n = line.find(']')
      acc += line[:n]
      if n != -1:
         for c in ['\n', '\t', 3*' ', 2*' ']:
            acc = acc.replace(c, ' ')
         yield acc.strip()
         acc = ''

from graphmod import ssys_pos, ssys_jmp

dotpos = dict()
for lin in input_blocks(stdin):
   if lin.find('--') != -1:
      continue

   if (nam := lin.split(' ')[0]) in ['graph', 'edge', 'node']:
      continue

   if nam[0] == '"' and nam[-1] =='"':
      nam = nam[1:-1]
   position = lin.split('pos="', 1)[1]
   position = position[:position.find('"')].split(',')
   x, y = float(position[0]), float(position[1])
   dotpos[nam] = vec(x, y)

count = 0
bbox, oldbb = bb(), bb()
for k, v in ssys_pos.items():
   if k[0] != '_':
      oldbb += v
      if count<3 and k not in dotpos and ssys_jmp[k] != {}:
         stderr.write('"' + k + '" not found in dot output. why ?\n')
         count +=1
         if count == 3:
            stderr.write("(won't be repeated)\n")

for k in dotpos:
   dotpos[k] *= 3.0/2.0
   if k[0] != '_':
      bbox += dotpos[k]
      if k not in ssys_pos:
         stderr.write('"' + k + '" not found in ssys. Bye !\n')
         exit(-1)

again = bb()

for k, v in dotpos.items():
   dotpos[k] += oldbb.mini() - bbox.mini()
   again += dotpos[k]
   if k[0] != '_':
      ssys_pos[k] = v
stderr.write(' -> '.join([str(oldbb), str(bbox), str(again)]) + '\n')
