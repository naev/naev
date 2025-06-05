#!/usr/bin/env python3


from os.path import basename
from geometry import transf, vec
from ssys import nam2base, starmap, fil_ET, spob_fil, vec_to_element, vec_from_element
from math import sin, pi
sm = starmap()


def _key(v):
   acc = 0
   if v[1] < 0:
      acc += 2
      v = -v
   return acc + (2 - v[0])

def mk_p(L):
   pi = [n for n, _k in sorted(enumerate(L), key = lambda t: (_key(t[1]), t[0]))]
   where = pi.index(0)
   return pi[where:] + pi[:where]

def sys_relax( sys ):
   p = fil_ET(sys)
   T = p.getroot()
   myname = nam2base(T.attrib['name'])

   mapvs = []
   sysvs = []
   names = []
   count = 0
   for f in T.findall('./jumps/jump'):
      dst = nam2base(f.attrib['target'])
      names.append(dst)
      e= f.find('pos')
      # should we require 'was_auto' ?
      if e is not None and 'was_auto' in e.attrib:
         mapvs.append((sm[dst] - sm[myname]).normalize())
         sysvs.append(vec_from_element(e).normalize())
         count += 1

   if count>0:
      nop = lambda v: v
      flip = nop
      pi1 = mk_p(mapvs)
      pi2 = mk_p(sysvs)

      if pi1 == pi2:
         # same permutation -> everything is fine
         pass
      elif pi1 == pi2[:1] + pi2[1:][::-1]:
         stderr.write('\033[32m' + basename(sys) + '" flipped !\033[0m\n')
         flip = lambda sysv: vec(-sysv[0], sysv[1])
      else:
         stderr.write('\033[33m' + basename(sys) + '" crossed : ')
         pi1 = [names[i] for i in pi1]
         pi2 = [names[i] for i in pi2]
         stderr.write(', '.join(pi1) + ' -> ' + ', '.join(pi2) + '\033[0m\n')
         return False

      acc = transf()

      for mapv, sysv in zip(mapvs, sysvs):
         t = mapv.normalize() / sysv.normalize()
         acc @= t
         #stderr.write(' t='+str(int(t.get_angle()*180/pi)).rjust(4)+'°')
         #stderr.write(' acc='+str(int(acc.get_angle()*180/pi)).rjust(4)+'°\n')

      acc /= count
      # in degrees
      eps = 0.2
      if abs(acc.vec) > sin(eps/180.0*pi) or flip != nop:
         func = lambda x: acc(flip(x))
         #stderr.write('final acc='+str(int(acc.get_angle()*180/pi)).rjust(4)+'°\n')
         for e in T.findall('./spobs/spob'):
            spfil = spob_fil(nam2base(e.text))
            p2 = fil_ET(spfil)
            f = p2.getroot().find('pos')
            vec_to_element(f, func(vec_from_element(f)))
            p2.write(spfil)

         for i in [ './jumps/jump/pos', './asteroids/asteroid/pos',
            './waypoints/waypoint']:
            for e in T.findall(i):
               vec_to_element(e, func(vec_from_element(e)))
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
