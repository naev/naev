#!/usr/bin/env python3



from ssys import sysnam2sys, starmap, sys_fil_ET
sm = starmap()


def sys_freeze( sys ):
   changed = False
   p = sys_fil_ET(sys)
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
   from sys import argv, exit, stderr
   from os.path import basename
   args = argv[1:]

   if '-h' in args or '--help' in args or args == []:
      stderr.write('usage:  ' + basename(argv[0]) + '  <file1> ..\n')
      stderr.write('  Freezes its input xml ssys files.\n')
      stderr.write('  The actually modified ssys files are output.\n')
      exit(0)

   for i in args:
      if sys_freeze(i):
         print(i)

