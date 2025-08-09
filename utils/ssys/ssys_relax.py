#!/usr/bin/env python3


from sys import stdout, stderr
from os.path import basename
from geometry import transf, vec
from ssys import nam2base, starmap, spob_fil, ssys_xml, vec_to_pos, pos_to_vec, xml_node, naev_xml
from math import sin, pi
from minimize_angle_stretch import relax_dir
from xml_name import end_xml_name

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
   p = ssys_xml(sys)
   T = p['ssys']
   myname = nam2base(T['@name'])

   mapvs, sysvs, names = [], [], []
   for f in T['jumps']['jump']:
      dst = nam2base(f['@target'])
      e = f.get('pos')
      # should we require 'was_auto' ?
      if e is not None and '@was_auto' in e:
         names.append(dst)
         mapvs.append((sm[dst] - sm[myname]).normalize())
         sysvs.append(pos_to_vec(e).normalize())

   if names:
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
            wrn += [('2', '[flipped]')]
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
         col = '3' if (len(pi1) > 2) else '2'
         if op == 'flipped' or col == '3':
            wrn += [(col, '[better ' + op + ']')]

      if cost > 0.5:
         wrn += [('3', '[badness ' + str((int)((100*cost)/2)) + '%]')]

      if wrn:
         wrn[0] = (wrn[0][0], '"' + basename(sys) + '": ' + wrn[0][1])
         stderr.write(' '.join(['\033[3'+str(i)+'m' + j + '\033[0m' for i, j in wrn]) + '\n')
         stderr.flush()

      if abs(alpha) > eps or flip != nop:
         func = lambda x: flip(x).rotate(alpha)
         for e in T['spobs']['spob']:
            spo = naev_xml(spob_fil(nam2base(e)))
            sp = spo['spob']
            sp['pos'] = vec_to_pos(func(pos_to_vec(sp['pos'])))
            spo.save()

         for t in ['jump', 'asteroid', 'waypoint']:
            for i, e in enumerate(T[t + 's'][t]):
               d, k = (T[t + 's'][t], i) if t == 'waypoint' else (e, "pos")
               # because of the list defect
               d[k] = xml_node(d[k]| vec_to_pos(func(pos_to_vec(d[k]))))
         p.save()
         return True
   return False

if __name__ == '__main__':
   from sys import argv, exit
   from os import fork, wait
   args = argv[1:]
   jobs = 1

   if '-h' in args or '--help' in args or not args:
      stderr.write(
         'usage:  ' + basename(argv[0]) + '[-j <n>]  [-v|-g]  <file1> ..\n'
         '  Relaxes its input xml ssys files.\n'
         '  If -j is set, uses <n> processes.\n'
         '  If -v is set, display information.\n'
         '  If -g is set, outputs the cost graph.\n'
      )
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
      # this one would interfere
      end_xml_name()
      while wait() != -1:
         pass
   except:
      pass
