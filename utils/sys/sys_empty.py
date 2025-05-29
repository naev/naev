#!/usr/bin/env python3


import xml.etree.ElementTree as ET

def is_empty( sys ):
   T = ET.parse(sys).getroot()

   if T.find('./general/nolanes') is None:
      return False

   for i in [
      './asteroids/asteroid/pos',
      './spobs/spob',
      './waypoints/waypoint'
   ]:
      if T.find(i) is not None:
         # has an object
         return False

   for e in T.findall('./jumps/jump/pos'):
      if 'was_auto' not in e.attrib:
         # has a fixed jump point
         return False

   # just in case ( should not happen )
   for e in T.findall('./tags/tag'):
      if e.text == 'tradelane':
         return False

   return True

if __name__ == '__main__':
   from sys import argv, exit, stderr, stdin
   from os.path import basename
   args = argv[1:]

   if '-h' in args or '--help' in args or args == []:
      stderr.write('usage:  ' + basename(argv[0]) + '  [-r]  -f | (<file1> ..)\n')
      stderr.write('  Lists the empty sys among its input xml ssys files.\n')
      stderr.write('  If -f is set, reads the list on stdin.\n')
      stderr.write('  If -r is set, list the non-empty sys.\n')
      exit(0)

   if rev:= '-r' in args:
      args.remove('-r')

   if '-f' in args:
      args.remove('-f')
      if len(args) > 1:
         stderr.write('ignored: ' + ', '.join(args) + '\n')
      args = map(lambda x: x.rstrip('\n'), stdin)

   for i in args:
      if is_empty(i) != rev:
         print(i)
