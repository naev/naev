#!/usr/bin/env python3



from ssys import nam2base, starmap, ssys_xml
sm = starmap()


def ssys_freeze( sys ):
   f = ssys_xml(sys)
   p = f['ssys']
   myname = nam2base(p['@name'])
   radius = p['general']['$radius']

   for e in p['jumps']['jump']:
      if 'autopos' in e:
         del e['autopos']
         dst = nam2base(e['@target'])
         v = (sm[dst] - sm[myname]).normalize()*radius
         e['pos'] = {
            '$@x': v[0],
            '$@y': v[1],
            '@was_auto': 'true'
         }
   return f.save(if_needed= True)


if __name__ == '__main__':
   from sys import argv, exit, stderr, stdin
   from os.path import basename
   args = argv[1:]

   if '-h' in args or '--help' in args or not args:
      stderr.write(
         'usage:  ' + basename(argv[0]) + '  -f | (<file1> ..)\n'
         '  Freezes its input xml ssys files.\n'
         '  The actually modified ssys files are output.\n'
         '  If -f is set, reads the list on stdin.\n'
      )
      exit(0)

   if '-f' in args:
      args.remove('-f')
      if len(args) > 1:
         stderr.write('ignored: ' + ', '.join(args) + '\n')
      args = map(lambda x: x.rstrip('\n'), stdin)

   for i in args:
      if ssys_freeze(i):
         print(i)
