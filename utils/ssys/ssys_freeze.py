#!/usr/bin/env python3



from ssys import nam2base, starmap, fil_ET, vec_to_element
sm = starmap()


def ssys_freeze( sys ):
   changed = False
   p = fil_ET(sys)
   T = p.getroot()
   myname = nam2base(T.attrib['name'])
   radius = float(T.find('general/radius').text)

   for e in T.findall('jumps/jump'):
      dst = nam2base(e.attrib['target'])
      for f in e.findall('autopos'):
         changed = True
         f.tag = 'pos'
         v = (sm[dst] - sm[myname]).normalize()*radius
         vec_to_element(f, v)
         f.set('was_auto','true')
   if changed:
      p.write(sys)
   return changed


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
