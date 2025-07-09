#!/usr/bin/env python3


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import stderr, argv, exit

if expe := '-e' in argv[1:]:
   argv.remove('-e')

if argv[1:] != []:
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + '\n'
      '  Reads a graph file on stdin, outputs a graph on stdout.\n'
      '  Intended as a postprocessing for neato output.\n'
   )
   exit(0)

from geometry import vec
from graphmod import sys_pos as pos, sys_jmp as E


# Smoothen tradelane

newp = dict()
for k in pos:
   if k[0] != '_':
      tln = [s for (s, t) in E[k] if 'tradelane' in t]
      if (n := len(tln)) > 1:
         if expe:
            acc = vec()
            count = 0
            for s in tln:
               acc2 = vec()
               count2 = 0
               for u, v in E[s]:
                  if u != k and 'tradelane' in v:
                     acc2 += pos[u]
                     count2 += 1
               if count2 == 0:
                  acc += pos[s]
               else:
                  acc += 1.5*pos[s] - 0.5*acc2/count2
               count += 1
            newp[k] = (pos[k] + acc) / (1.0 + count)
         else:
            p = sum([pos[s] for s in tln], vec())
            newp[k] = (pos[k] + 0.25 * p) / (1.0 + 0.25 * count)

for k, v in newp.items():
   pos[k] = v
