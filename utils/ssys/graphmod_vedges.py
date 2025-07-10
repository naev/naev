#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, stderr
from graph_vaux import color_values

if argv[1:] != []:
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + '\n'
      '  Reads a graph file on stdin, outputs a graph on stdout.\n'
      '  Adds a few virtual edges.\n'
   )
   exit(0)

# In the form: (from, to [, length])
virtual_edges = [
   ('ngc902', 'ngc4087'),
]

from graphmod import sys_pos as V, sys_jmp as E
from virtual_edges import add_virtual_edges

add_virtual_edges(E, virtual_edges)
