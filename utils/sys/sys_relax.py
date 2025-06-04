#!/usr/bin/env python3


from geometry import transf, vec
from ssys import nam2base, starmap, fil_ET, spob_fil, vec_to_element, vec_from_element
from math import sin, pi
sm = starmap()


def sys_relax( sys ):
   p = fil_ET(sys)
   T = p.getroot()
   myname = nam2base(T.attrib['name'])

   acc = transf()
   count = 0

   for f in T.findall('./jumps/jump'):
      dst = nam2base(f.attrib['target'])
      for e in f.findall('pos'):
         if 'was_auto' in e.attrib:
            mapv = sm[dst] - sm[myname]
            sysv = vec_from_element(e)
            t = mapv.normalize() / sysv.normalize()
            acc @= t
            #stderr.write(' t='+str(int(t.get_angle()*180/pi)).rjust(4)+'°')
            #stderr.write(' acc='+str(int(acc.get_angle()*180/pi)).rjust(4)+'°\n')
            count += 1

   if count>0:
      acc /= count
      # in degrees
      eps = 0.2
      if abs(acc.vec) > sin(eps/180.0*pi):
         #stderr.write('final acc='+str(int(acc.get_angle()*180/pi)).rjust(4)+'°\n')
         for e in T.findall('./spobs/spob'):
            spfil = spob_fil(nam2base(e.text))
            p2 = fil_ET(spfil)
            f = p2.getroot().find('pos')
            vec_to_element(f, acc(vec_from_element(f)))
            p2.write(spfil)

         for i in [ './jumps/jump/pos', './asteroids/asteroid/pos',
            './waypoints/waypoint']:
            for e in T.findall(i):
               vec_to_element(e, acc(vec_from_element(e)))
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
