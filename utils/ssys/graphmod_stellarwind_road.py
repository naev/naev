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


from geometry import vec, symmetry
from graphmod import ssys_pos, ssys_jmp

L = [
   ('c43', ('c59', -0.5), 'hades'),
   ('c43', ('polack', 1.0 - 0.25), ('chloe', 1 + 0.25)),
   ('polack', 'wolf'),
   (('terminus', 0.8), 'flok', ('wolf', 2)),
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

   road = ['c59'] + names + ['sollav']
   for i, j in zip(road[:-1], road[1:]):
      ssys_jmp[i][j] = ['new'] + (['hidden'] if i == road[0] else [])
      ssys_jmp[j][i] = ['new'] + (['hidden'] if j == road[-1] else [])

   ssys_jmp['mason']['c59'] = ['new', 'hidden']
   ssys_jmp['c59']['mason'] = ['new', 'hidden']

   ssys_jmp[road[3]] |= {'chloe': ['new']}
   ssys_jmp[road[5]] |= {'flok': ['new']}
   ssys_jmp[road[7]] |= {'yarn': ['new'], 'delta_polaris': ['new']}
   ssys_jmp[road[9]] |= {'logania': ['new'], 'palejos': ['new'], 'ngc4746': ['new']}
   ssys_jmp['ngc4746'] |= {road[9]: ['new', 'hidden']}
   ssys_jmp[road[11]] |= {'octavian': ['new']}
   ssys_jmp[road[13]] |= {'olympus': ['new']}

   ssys_pos['wolf'] = symmetry(ssys_pos['chloe'], ssys_pos['zemur']) (ssys_pos['wolf'])
   ssys_pos['defa'] += 0.05 * (ssys_pos['taiomi'] - ssys_pos['defa'])

   v = ssys_pos['lalande'] - ssys_pos['pilatis']
   for s in ['draconis', 'pilatis', 'levo']:
      ssys_pos[s] += 0.2 * v

   from smoothen import smoothen_induced
   from graph_vaux import ssys_others
   stellars = set(road)
   ssys_pos |= smoothen_induced(ssys_pos, ssys_jmp, stellars)

   stellars |= {k for k in ssys_pos if 'stellarwind' in ssys_others(ssys_pos, k)
      and ssys_pos[k][1] < ssys_pos['goddard'][1]}
   ssys_pos |= smoothen_induced(ssys_pos, ssys_jmp, stellars)

   around_defa = {'draconis', 'pilatis', 'defa', 'oberon', 'shikima'}
   res = smoothen_induced(ssys_pos, ssys_jmp, around_defa)
   for k in {'pilatis', 'defa'}:
      ssys_pos[k] = res[k]
