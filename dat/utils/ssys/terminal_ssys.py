#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import stderr, argv, exit

if argv[1:]:
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + '\n'
      '  Reads a graph file on stdin. Outputs the list of term ssys.\n'
   )
   exit(0)

from graphmod import ssys_pos, ssys_jmp, no_graph_out
no_graph_out()

print (' '.join([i for i in ssys_pos if len(ssys_jmp[i]) <= 1]))
