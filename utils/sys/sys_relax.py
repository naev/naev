#!/usr/bin/env python3


import xml.etree.ElementTree as ET
from ssys import sysnam2sys, starmap
sm = starmap()


def sys_relax( sys ):
   changed = False
   T = ET.parse(sys).getroot()

   changed = True
   for e in T.findall('./jumps/jump/pos'):
      if 'was_auto' in e.attrib:
         #TODO
         pass
   return changed

if __name__ == '__main__':
   from sys import argv, exit, stderr
   from os.path import basename
   args = argv[1:]

   if '-h' in args or '--help' in args or args == []:
      stderr.write('usage:  ' + basename(argv[0]) + '  <file1> ..\n')
      stderr.write('  Relaxes its input xml ssys files.\n')
      exit(0)

   for i in args:
      if sys_relax(i):
         print(i)
