#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, stderr

if JUST_LIST := '-L' in argv[1:]:
   argv.remove('-L')

if argv[1:]:
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + ' [-L]\n'
      '  Reads a graph file on stdin, outputs a graph on stdout.\n'
      '  Graphmod intended to insert new stellar wind road systems.\n'
      '  If -L is set, only outputs the list of sys it would modify.\n'
   )
   exit(0)


from geometry import vec
from graphmod import ssys_pos, ssys_jmp

L = [
   ('c43', 'c59', 'hades'),
   ('c43', 'hades', 'polack', 'chloe'),
   ('polack', 'chloe', 'terminus'),
   ('terminus', 'flok', ('wolf', 1.33)),
   ('zemur', 'flok', 'sirou'),
   ('griffin', 'sirou', 'yarn'),
   ('yarn', 'griffin', 'jade', 'blackwell'),
   ('logania', 'palejos', 'jade', 'blackwell'),
   (('swr8', -1), 'palejos', 'logania'),
   (('swr9', 1.66), ('swr8', -0.66)),
   ('procyon', 'swr10'),
   ('procyon', 'olympus'),
   ('olympus', 'vost', 'hargen', 'regas'),
   ('hargen', 'regas', ('swr13', -1), ('sollav', 0.2)),
]

output = {}
for i, t in enumerate(L, 1):
   wl = map(lambda s: (s, 1) if isinstance(s, str) else s, t)
   wl = [(vec( (output | ssys_pos) [s] ), w) for s, w in wl]
   output['swr'+str(i)] = sum([v * w for v, w in wl], vec()) / sum([w for _, w in wl])

if JUST_LIST:
   print(' '.join(map(str,output.keys())))
else:
   ssys_pos |= output
   ssys_pos.aux |= {i: ["default::stellarwind:spoiler:unused", 'S'+i[1:]] for i in output}
   road = ['mason'] + ['swr' + str(i) for i, _ in enumerate(L, 1)] + ['sollav']
   for i, j in zip(road[:-1], road[1:]):
      ssys_jmp[i][j] = ['new', 'hidden']

   for sys in [road[0], road[-1]]:
      if ':' not in ssys_pos.aux[sys][0]:
         ssys_pos.aux[sys][0] += ':'
      ssys_pos.aux[sys][0] += ':stellarwind'
