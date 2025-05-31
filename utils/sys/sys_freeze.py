#!/usr/bin/env python3



from ssys import sysnam2sys, starmap, fil_ET
sm = starmap()


def sys_freeze( sys ):
   changed = False
   p = fil_ET(sys)
   T = p.getroot()
   myname = sysnam2sys(T.attrib['name'])
   radius = float(T.find('general/radius').text)

   for e in T.findall('jumps/jump'):
      dst = sysnam2sys(e.attrib['target'])
      for f in e.findall('autopos'):
         changed = True
         f.tag = 'pos'
         v = (sm[dst] - sm[myname]).normalize()*radius
         for k, v in v.to_dict().items():
            f.set(k, str(v))
         f.set('was_auto','true')
   if changed:
      p.write(sys)
   return changed


if __name__ == '__main__':
   from sys import argv, exit, stderr, stdin
   from os.path import basename
   args = argv[1:]

   if '-h' in args or '--help' in args or args == []:
      stderr.write('usage:  ' + basename(argv[0]) + '  -f | (<file1> ..)\n')
      stderr.write('  Freezes its input xml ssys files.\n')
      stderr.write('  The actually modified ssys files are output.\n')
      stderr.write('  If -f is set, reads the list on stdin.\n')
      exit(0)

   if '-f' in args:
      args.remove('-f')
      if len(args) > 1:
         stderr.write('ignored: ' + ', '.join(args) + '\n')
      args = map(lambda x: x.rstrip('\n'), stdin)

   for i in args:
      if sys_freeze(i):
         print(i)

