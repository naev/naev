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
   ('terminus', 'flok', ('wolf', 1.5)),
   (('zemur', 0.8), 'flok', 'sirou'),
   ('griffin', 'sirou', 'yarn'),
   ('yarn', 'griffin', 'jade', 'blackwell'),
   ('logania', 'palejos', 'jade', 'blackwell'),
   (('sw8', -1), 'palejos', 'logania'),
   ('ngc4746', 'ngc7533', 'dendria'),
   ('procyon', 'sw10', ('olympus', 0.1)),
   ('procyon', 'olympus'),
   ('olympus', 'vost', 'hargen', 'regas'),
   ('hargen', 'regas', ('sw13', -1), ('sollav', 0.2), ('pultatis', 0.3)),
]

output = {}
names = ['sw' + str(i) for i, _ in enumerate(L, 1)]
for n, t in zip(names, L):
   wl = map(lambda s: (s, 1) if isinstance(s, str) else s, t)
   wl = [(vec( (output | ssys_pos) [s] ), w) for s, w in wl]
   output[n] = sum([v * w for v, w in wl], vec()) / sum([w for _, w in wl])

if JUST_LIST:
   print(' '.join(output.keys()))
else:
   ssys_pos |= output
   ssys_pos.aux |= {i: ["default::stellarwind:spoiler:new", i.upper().replace('W','W-')] for i in output}
   road = ['mason'] + names + ['sollav']
   for i, j in zip(road[:-1], road[1:]):
      ssys_jmp[i][j] = ['new'] + (['hidden'] if i == road[0] else [])
      ssys_jmp[j][i] = ['new'] + (['hidden'] if j == road[-1] else [])

   ssys_pos.aux['mason'][0] = ssys_pos.aux['mason'][0].replace(':stellarwind',':stellarwind:northstellarwind:update')
   ssys_pos.aux['sollav'][0] = ssys_pos.aux['sollav'][0].replace(':stellarwind',':stellarwind:southstellarwind:update')
