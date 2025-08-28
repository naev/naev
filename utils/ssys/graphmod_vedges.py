#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, stderr
from graph_vaux import color_values

if argv[1:]:
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + '\n'
      '  Reads a graph file on stdin, outputs a graph on stdout.\n'
      '  Adds a few virtual edges.\n'
   )
   exit(0)

# In the form: (from, to [, length])
virtual_edges = [
   ('ngc902', 'ngc4087'),
   ('ngc728', 'ngc1872'),
   ('brumerebus', 'antlejos'),
   ('kiwi', 'suna'),
   ('pellmell', 'fulcrum'),
   ('nasona', 'oriantis'),
   ('tau_ceti', 'arcanis'),
   ('ngc8338', 'unicorn'),
   #('carrza', 'tepvin'),
   #('tepvin', 'hakoi'),
]

from graphmod import ssys_jmp
from virtual_edges import add_virtual_edges

add_virtual_edges(ssys_jmp, virtual_edges)
