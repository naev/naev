#!/usr/bin/env python3


from geometry import transf, vec
from ssys import sysnam2sys, spobnam2spob, starmap, fil_ET, spob_fil
from math import sin, pi
sm = starmap()


def sys_relax( sys ):
   p = fil_ET(sys)
   T = p.getroot()
   myname = sysnam2sys(T.attrib['name'])

   acc = transf()
   count = 0

   for f in T.findall('./jumps/jump'):
      dst = sysnam2sys(f.attrib['target'])
      for e in f.findall('pos'):
         if 'was_auto' in e.attrib:
            mapv = sm[dst] - sm[myname]
            sysv = vec(float(e.attrib['x']), float(e.attrib['y']))
            acc @= mapv.normalize() / sysv.normalize()
            count += 1

   if count>0:
      acc /= count
      print(acc)
      # in degrees
      eps = 0.2
      if abs(acc.vec) > sin(eps/180.0*pi):
         for e in T.findall('./spobs/spob'):
            spfil = spob_fil(sysnam2sys(e.text))
            p2 = fil_ET(spfil)
            f = p2.getroot().find('pos')
            sysv = acc(vec(float(f.attrib['x']), float(f.attrib['y'])))
            f.set('x', str(sysv[0]))
            f.set('y', str(sysv[1]))
            p2.write(spfil)

         for i in [ './jumps/jump/pos', './asteroids/asteroid/pos',
            './waypoints/waypoint']:
            for e in T.findall(i):
               sysv = acc(vec(float(e.attrib['x']), float(e.attrib['y'])))
               e.set('x', str(sysv[0]))
               e.set('y', str(sysv[1]))
         p.write(sys)
         return True
   return False

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
