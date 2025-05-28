#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, exit, stderr
from os.path import basename
args = argv[1:]

if '-h' in args or '--help' in args or args == []:
   stderr.write('usage:  ' + basename(argv[0]) + '  <file1> ..\n')
   stderr.write('  Freezes its input xml ssys files.\n')
   exit(0)

from ssys import starmap, sysnam2sys, sys_fil_ET

sm = starmap()

for i in args:
   changed = False
   p = sys_fil_ET(i)
   T = p.getroot()
   myname = sysnam2sys(T.attrib["name"])

   for e in T.findall("general/radius"):
      radius = float(e.text)
      break

   for e in T.findall("jumps/jump"):
      dst = sysnam2sys(e.attrib['target'])
      for f in e.findall("autopos"):
         changed = True
         f.tag = "pos"
         v = (sm[dst] - sm[myname]).normalize()*radius
         for k, v in v.to_dict().items():
            f.set(k, str(v))
         f.set('was_auto','true')

   if changed:
      p.write(i)
      stderr.write('froze <' + basename(i) + '>\n')

