#!/usr/bin/env python3


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import stderr, argv, exit

if JUST_LIST := '-L' in argv[1:]:
   argv.remove('-L')

if argv[1:]:
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + ' [-L]\n'
      '  Reads a graph file on stdin, outputs a graph on stdout.\n'
      '  Intended as a postprocessing for neato output.\n'
      '  If -L is set, only outputs the list of sys it would modify.\n'
   )
   exit(0)

from geometry import symmetry
from graphmod import ssys_pos, ssys_jmp, no_graph_out
from smoothen import smoothen, smoothen_induced, circleify
from graph_vaux import ssys_others

L = {'eiderdown', 'gilligans_tomb', 'adraia', 'vanir', 'botarn', 'monogram', 'kraft', 'pike'}
sirius_circle = {'palovi',} | L
tradelane = { k: {s for (s, t) in ssys_jmp[k].items() if 'tradelane' in t and not {k, s} < L} for k in ssys_pos }

rem_tl = lambda l, s: {k: {} if k in s else (v - s) for k, v in l.items()}
tradelane = rem_tl(tradelane, {'point_zero'})

stellars = {k for k in ssys_pos if 'stellarwind' in ssys_others(ssys_pos, k)} - {'c59'}
abh_circle = {'ngc11935', 'ngc5483', 'ngc7078', 'ngc7533', 'octavian', 'copernicus', 'ngc13674', 'ngc1562', 'ngc2601'}

trad_l_pos = smoothen(ssys_pos, tradelane)
stel_w_pos = smoothen_induced(
   ssys_pos | {'pilatis': symmetry(ssys_pos['defa'])(ssys_pos['oberon'])},
   ssys_jmp, stellars, hard= True)

if JUST_LIST:
   no_graph_out()
   print('\n'.join(trad_l_pos.keys() | stel_w_pos.keys() | sirius_circle | abh_circle))
else:
   ssys_pos |= circleify(ssys_pos, abh_circle, 'anubis_black_hole')
   ssys_pos |= circleify(ssys_pos, sirius_circle, 'suna', hard= True)
   ssys_pos |= stel_w_pos | trad_l_pos
