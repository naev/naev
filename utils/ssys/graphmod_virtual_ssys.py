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

from geometry import vec
from graphmod import ssys_pos as pos, ssys_jmp

# average edge length.
total = 0.0
count = 0
for k in pos:
   if k[0] != '_':
      for n in [s for (s, t) in ssys_jmp[k].items() if 'tradelane' in t]:
         total += (pos[n] - pos[k]).size()
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
