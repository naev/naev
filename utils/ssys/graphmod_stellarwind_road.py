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
   ('c00', 'c14', 'c28', 'c43', 'c59'),
   (('swr1', -1), 'c14', 'c43'),
   ('ngc11718', 'swr2'),
   ('paradox', 'zemur', 'wolf', 'chloe'),
   ('paradox', 'zemur', 'delta_polaris'),
   ('yarn', 'griffin', 'jade'),
   ('logania', 'palejos', 'jade'),
   (('swr7', -1), 'palejos', 'logania'),
   (('swr8', 2), ('swr7', -1)),
   ('ux', 'octavian'),
   ('olympus', 'ganth', 'dendria', 'regulus'),
   ('attaria', 'pultatis', 'provectus_nova', 'ganth', 'regulus'),
   ('pultatis', 'provectus_nova', 'limbo', 'sollav'),
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
   road = ['mason'] + ['swr' + str(i) for i, _ in enumerate(L, 1)] + ['fried']
   for i, j in zip(road[:-1], road[1:]):
      ssys_jmp[i][j] = ['new']
