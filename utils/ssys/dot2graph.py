#!/usr/bin/env python3


from sys import stdin, stderr, argv, exit
from geometry import bb, vec
import os
import xml.etree.ElementTree as ET
from ssys import getpath, PATH



if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

if argv[1:] != []:
   stderr.write('usage: ' + argv[0].split('/')[-1] + '\n')
   stderr.write('  Reads a dot file on stdin, writes the graph on stdout.\n')
   exit(0)

# for comparison with new values, we need the old ones.
def ssys_pos( ):
   def all_ssys( args = None ):
      path = os.path.join(PATH, 'ssys')
      for arg in os.listdir(path):
         if arg[-4:] == '.xml':
            yield arg[:-4], os.path.join(path, arg)

   pos = {}
   for bname, filename in all_ssys():
      T=ET.parse(filename).getroot()

      try:
         e = T.find('pos')
         pos[bname] = (e.attrib['x'], e.attrib['y'])
      except:
         stderr.write('no position defined in "' + bname + '"\n')

   return pos

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

pos = dict()
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
   pos[nam] = vec(x, y)


oldpos = ssys_pos()

bbox, oldbb = bb(), bb()
for k in pos:
   pos[k] *= 3.0/2.0
   if k[0] != '_':
      bbox += pos[k]
      if k not in oldpos:
         stderr.write('"' + k + '" not found in ssys. why ?\n')
      else:
         oldbb += oldpos[k]


again = bb()

for k, v in pos.items():
   pos[k] += oldbb.mini() - bbox.mini()
   again += pos[k]
   if k[0] != '_':
      v = round(v, 9)
      print(k, v[0], v[1])
stderr.write(' -> '.join([str(oldbb), str(bbox), str(again)]) + '\n')
