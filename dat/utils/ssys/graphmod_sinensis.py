#!/usr/bin/env python3


from sys import stderr, argv, exit


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

if argv[1:]:
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + '\n'
      '  Reads a graph file on stdin, outputs a graph on stdout.\n'
      '  Intended as a postprocessing.\n'
   )
   exit(0)


from geometry import bb, vec, segment, symmetry
from graphmod import ssys_pos as pos, ssys_jmp as E

v = pos['ngc9017'] - pos['ngc10180']
pos['ngc10180'] += v
pos['ngc9017'] += v
pos['ngc2948'] += v
pos['ngc1098'] -= pos['ngc9415'] - pos['westhaven']

u = 0.2 * (pos['ngc10180']-pos['sinensis'])
pos['sinensis'] += u
pos['ngc2948'] += u + 0.2 * (pos['ngc1098']-pos['ngc9017'])

v= ((pos['syndania']-pos['jommel']) + (pos['haered'] - pos['scholzs_star']))/2.0
v /= 5.0

proteron = ['leporis', 'hystera', 'korifa', 'apik', 'telika', 'mida', 'ekta', 'akra']

for s in ['syndania', 'nirtos', 'sagittarius', 'hopa', 'scholzs_star',
   'veses', 'alpha_centauri', 'padonia',
   'urillian', 'baitas', 'protera', 'tasopa']:
   pos[s] += v

for s in ['haered'] + proteron:
   pos[s]+= 2 * v
