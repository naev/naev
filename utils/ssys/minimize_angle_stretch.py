#!/usr/bin/env python3


from sys import stderr
from geometry import vec
from math import pi
import heapq

# [0:2.PI] -> [0,2]
# max(abs(slope)) = 1

fact = 0.2
def cost_func( sys_dirs, sm_dirs ):
   f = 1.0 / len(sys_dirs)
   L = [(x.normalize(f), y.normalize((f*(1.0+fact)))) for x, y in zip(sys_dirs, sm_dirs)]
   return lambda a: sum([(x.rotate_rad(a)-y).size() for x, y in L]) - fact

def best( cost, a, b ):
   if b[0] < a[0]:
      a, b = b, a
   return a[1] - ((b[0]-a[0])-(b[1]-a[1])) / 2.0

def _relax_dir( sys_dirs, sm_dirs, eps = 0.00001 ):
   cost = cost_func(sys_dirs, sm_dirs)
   mi = (0, cost(0))
   points = [mi, (2*pi, mi[1])]
   inter = [tuple(points)]
   inter = [(best(cost,*t),) + t for t in inter]
   heapq.heapify(inter)
   while len(inter) > 0:
      (v, a, b) = heapq.heappop(inter)
      if v > mi[1] or b[0]-a[0] < eps:
         continue
      m = (a[0] + b[0]) / 2.0
      c = (m, cost(m))
      points.append(c)
      if c[1] < mi[1]:
         mi = c
      heapq.heappush(inter, (best(cost, a, c), a, c))
      heapq.heappush(inter, (best(cost, c, b), c, b))

   points.sort()
   inter = [(a, b) for (be, a, b) in inter]
   return points, inter, mi, cost

def _debug_relax( p, inter, t, out, cost ):
   import subprocess
   import tempfile

   (x, _y) = t
   plot = tempfile.NamedTemporaryFile(mode = 'w+t', suffix = '.plot')
   dat  = tempfile.NamedTemporaryFile(mode = 'w+t', suffix = '.dat')
   dat2 = tempfile.NamedTemporaryFile(mode = 'w+t', suffix = '_c.dat')
   png  = out + '.png'

   for i in range(500+1):
      f = 2.0*pi*i/500
      dat2.write(str(f) + ' ' + str(cost(f)) + ' ' + '\n')
   dat2.flush()

   for t in p:
      dat.write(' '.join(map(str,t)) + '\n')
   dat.flush()

   plot.write('\n'.join([
      'set terminal pngcairo size 1280,720 enhanced', '',
      'set output "' + png + '"',
      'set key outside', 'set yrange [0:2]',
      'set xrange [0:2*' + str(pi) + ']', 'set termoption dashed',
      'set grid',
      'set arrow from ' + str(x) + ', graph 0 to ' + str(x) + ', graph 1 nohead',
      'plot "' + dat2.name + '" using 1:2 w lines t "cost(angle)",\\',
      '     "' + dat.name + '" using 1:2 w linespoints t ""\\',
      ''
   ]))
   plot.flush()
   subprocess.run(['gnuplot', plot.name])
   stderr.write('\033[30;1m<' + png + '>' + '\033[0m\n')

# returns angle (deg), cost (lower better)
def relax_dir( sys_dirs, sm_dirs, eps = 0.00001, debug = False, quiet = False ):
   p, i, mi, co = _relax_dir(sys_dirs, sm_dirs, eps)
   if (not quiet) or debug:
      stderr.write(' \033[30;1mpoints: ' + str(len(p)) + ' ')
      stderr.write('badness: ' + str(int(round(mi[1]/2.0*100.0))) + '%\033[0m')
      stderr.write('\n' if not debug else ' ')

   if debug:
      _debug_relax(p, i, mi, debug, co)

   alpha = mi[0] / pi * 180.0
   if alpha > 180.0:
      alpha -= 360.0
   return alpha, mi[1]

if __name__ == '__main__':
   from random import random

   N = 3
   sys, sm = [], []
   for _ in range(N):
      sys.append(vec(1,0).rotate(360.0 * random()))
      sm.append(vec(1,0).rotate(360.0 * random()))

   relax_dir( sys, sm, debug = 'out')
