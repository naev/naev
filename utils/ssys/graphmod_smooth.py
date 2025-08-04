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

from graphmod import ssys_pos, ssys_jmp, no_graph_out
from smoothen import smoothen, circleify

L = { 'eiderdown', 'gilligans_tomb', 'adraia', 'vanir', 'botarn', 'monogram', 'kraft', 'pike'}
CIR = {'palovi',} | L
neigh = { k: {s for (s, t) in ssys_jmp[k].items() if 'tradelane' in t and not {k, s} < L} for k in ssys_pos }
ABH = {'ngc11935', 'ngc5483', 'ngc7078', 'ngc7533', 'octavian', 'copernicus', 'ngc13674', 'ngc1562', 'ngc2601'}

if JUST_LIST:
   no_graph_out()
   print('\n'.join({k for k in neigh if neigh[k]}|CIR))
else:
   ssys_pos |= circleify(ssys_pos, ABH, 'anubis_black_hole')
   ssys_pos |= circleify(ssys_pos, CIR, 'suna')
   ssys_pos |= smoothen(ssys_pos, neigh)
