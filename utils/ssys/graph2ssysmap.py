#!/usr/bin/env python3


import os
from sys import stderr, exit
import xml.etree.ElementTree as ET
from ssys import nam2base, getpath, PATH


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import argv, exit, stdout, stderr, stdin

help_f = '-h' in argv or '--help' in argv[1:]
if help_f or (argv[1:] and do_write):
   msg = lambda s: (stdout if help_f else stderr).write(s + '\n')
   DOC = [
      'usage:  ' + os.path.basename(argv[0]),
      '  Updates ssys/*.xml according to the graph provided in input.'
   ]
   for l in DOC:
      msg(l)
   exit(0 if help_f else 1)

from graphmod import ssys_pos, no_graph_out
no_graph_out()

from ssys import fil_ET
for n, (x, y) in ssys_pos.items():
   name = os.path.join(PATH, 'ssys', n + '.xml')
   T = fil_ET(name)
   e = T.getroot().find('pos')
   e.attrib['x'], e.attrib['y'] = str(x), str(y)
   T.write(name)
