#!/usr/bin/env python3


from sys import stdout, stderr
from os.path import basename
from geometry import transf, vec
from ssys import nam2base, starmap, fil_ET, spob_fil, vec_to_element, vec_from_element
from math import sin, pi
from minimize_angle_stretch import relax_dir

sm = starmap()


def _key( v ):
   acc = 0
   if v[1] < 0:
      acc += 2
      v = -v
   return acc + (2 - v[0])

def mk_p( L ):
   pi = [n for n, _k in sorted(enumerate(L), key = lambda t: (_key(t[1]), t[0]))]
   where = pi.index(0)
   return pi[where:] + pi[:where]

def ssys_relax( sys, quiet = True, graph = False ):
   p = fil_ET(sys)
   T = p.getroot()
   myname = nam2base(T.attrib['name'])

   mapvs, sysvs, names = [], [], []
   for f in T.findall('./jumps/jump'):
      dst = nam2base(f.attrib['target'])
      e= f.find('pos')
      # should we require 'was_auto' ?
      if e is not None and 'was_auto' in e.attrib:
         names.append(dst)
         mapvs.append((sm[dst] - sm[myname]).normalize())
         sysvs.append(vec_from_element(e).normalize())

   if names != []:
      nop = lambda v: v
      flip = nop
      pi1, pi2 = mk_p(mapvs), mk_p(sysvs)
      # more than max possible cost <= 2.0
      eps = 0.0001
      out = sys.replace('.xml', '') if graph else None

      wrn = []

      if pi1 == pi2: # same permutation -> everything is fine
         # If only two jumps, we can flip to try improving the result.
         try_flipped, try_unflipped = (len(pi1) == 2), True
      else:
         if pi1 == pi2[:1] + pi2[1:][::-1]:
            wrn += [('2', 'flipped')]
            try_flipped, try_unflipped = True, False
         else:
            pi1 = [names[i] for i in pi1]
            pi2 = [names[i] for i in pi2]
            wrn += [('3', 'crossed: ' + ', '.join(pi1) + ' -> ' + ', '.join(pi2))]
            try_flipped, try_unflipped = True, True

      if try_unflipped:
         alpha, cost = relax_dir(sysvs, mapvs,
            eps = eps/10.0, debug = out, quiet = quiet)
      else:
         alpha, cost = 360.0, 3.0

      if try_flipped:
         outf = None if out is None else (out + '_f')
         _flip = lambda v: vec(-v[0], v[1])
         alpha_f, cost_f = relax_dir([_flip(v) for v in sysvs], mapvs,
            eps = eps/10.0, debug = outf, quiet = quiet)

         if cost_f < cost:
            alpha, cost, flip = alpha_f, cost_f, _flip

      if try_flipped and try_unflipped:
         op = 'unflipped' if flip is nop else 'flipped'
         col = '3' if len(pi1) > 2 else '2'
         if op == "flipped" or col == 3:
            wrn += [(col, '[better ' + op + ']')]

      if cost > 0.5:
         wrn += [('3', 'badness ' + str((int)((100*cost)/2)) + '%')]

      if wrn != []:
         wrn[0] = (wrn[0][0], '"' + basename(sys) + '": ' + wrn[0][1])
         stderr.write(''.join(['\033[3'+str(i)+'m' + j + '\033[0m\n' for i, j in wrn]))
         stderr.flush()

      if abs(alpha) > eps or flip != nop:
         func = lambda x: flip(x).rotate(alpha)
         for e in T.findall('./spobs/spob'):
            spfil = spob_fil(nam2base(e.text))
            p2 = fil_ET(spfil)
            f = p2.getroot().find('pos')
            vec_to_element(f, func(vec_from_element(f)))
            p2.write(spfil)

         for i in ['./jumps/jump/pos', './asteroids/asteroid/pos', './waypoints/waypoint']:
            for e in T.findall(i):
               vec_to_element(e, func(vec_from_element(e)))
         p.write(sys)
         return True
   return False

if __name__ == '__main__':
   from sys import argv, exit
   from os import fork, wait
   args = argv[1:]
   jobs = 1

   if '-h' in args or '--help' in args or args == []:
      stderr.write('usage:  ' + basename(argv[0]) + '[-j <n>]  [-v|-g]  <file1> ..\n')
      stderr.write('  Relaxes its input xml ssys files.\n')
      stderr.write('  If -j is set, uses <n> processes.\n')
      stderr.write('  If -v is set, display information.\n')
      stderr.write('  If -g is set, outputs the cost graph.\n')
      exit(0)

   if verbose:= '-v' in args:
      args.remove('-v')

   if graph:= '-g' in args:
      args.remove('-g')

   if '-j' in args:
      i = args.index('-j')
      args.pop(i)
      try:
         jobs = int(args[i])
         args.pop(i)
      except:
         stderr.write('-j: int expected after.\n')
         exit(1)

   for i in range(jobs):
      if fork() == 0:
         for n, ssys in enumerate(args):
            if n % jobs == i:
               if ssys_relax(ssys, quiet = not verbose, graph = graph):
                  print(ssys)
                  stdout.flush()
         exit(0)
   try:
      while wait() != -1:
         pass
   except:
      pass
