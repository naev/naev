#!/usr/bin/env python3

from ssys import ssys_xml


def ssys_empty( sys ):
   T = ssys_xml(sys, read_only= True)['ssys']

   if not('general' in T and 'nolanes' in T['general']):
      return False

   for i in ['asteroid', 'spob', 'waypoint']:
      if len(T[i + 's'][i]):
         return False

   for e in T['jumps']['jump']:
      if 'pos' in e and '@was_auto' not in e:
         return False

   return True


if __name__ == '__main__':
   from sys import argv, exit, stderr, stdin
   from os.path import basename
   args = argv[1:]

   if '-h' in args or '--help' in args or not args:
      stderr.write(
         'usage:  ' + basename(argv[0]) + '  [-r]  -f | (<file1> ..)\n'
         '  Lists the empty ssys among its input xml ssys files.\n'
         '  If -r is set, list the non-empty ssys.\n'
         '  If -f is set, reads the list on stdin.\n'
      )
      exit(0)

   if rev:= '-r' in args:
      args.remove('-r')

   if '-f' in args:
      args.remove('-f')
      if len(args) > 1:
         stderr.write('ignored: ' + ', '.join(args) + '\n')
      args = map(lambda x: x.rstrip('\n'), stdin)

   for i in args:
      if ssys_empty(i) != rev:
         print(i)
