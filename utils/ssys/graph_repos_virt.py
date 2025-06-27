#!/usr/bin/env python3


from sys import stdin, stderr, argv, exit
from geometry import bb, vec, segment


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

if argv[1:] != []:
   stderr.write('usage: ' + argv[0].split('/')[-1] + '\n')
   stderr.write('  Reads a graph file on stdin, outputs a graph on stdout.\n')
   stderr.write('  Intended as a postprocessing for neato output.\n')
   exit(0)

pos = {}
for inp in stdin:
   if (line := inp.strip()) != '':
      bname, x, y = tuple(line.split(' '))
      pos[bname] = vec(x, y)


# Smoothen tradelane

from ssys_graph import xml_files_to_graph
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


# average edge length.

total = 0.0
count = 0
for k in pos:
   if k[0] != '_':
      for n in [s for (s, _) in E[k] if (s in tradelane)]:
         total += (pos[n]-pos[k]).size()
         count += 1
avg = total / count


# Position virtual systems

v = vec(avg, 0) * 0.4
for f, t, a in [
   ('beeklo',     'crimson_gauntlet',        120),
   ('anrique',    'test_of_renewal',         90),
   ('anarbalis',  'test_of_purification',    -60),
   ('churchill',  'test_of_alacrity',        0),
   ('ulysses',    'test_of_enlightenment',   135),
   ('aesir',      'test_of_devotion',        135),
]:
   pos[t] = pos[f] + v.rotate(a)


# Output it

for k, v in pos.items():
   if k[0] != '_':
      v = round(v, 9)
      print(k, v[0], v[1])
