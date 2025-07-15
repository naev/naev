#!/usr/bin/env python3

SHOW_VIRTUAL = True

from decorators import decorators
decorators = {s: (d, -i*0.1) for i, (d, s) in enumerate(decorators.items())}

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import argv, stdin, stderr, exit
from math import log, sqrt
import subprocess
import os

silent = False
pov_out = None

if no_preview := '-n' in argv[1:]:
   argv.remove('-n')

if show_decorators := '-d' in argv[1:]:
   argv.remove('-d')

if '-H' in argv[1:]:
   argv.remove('-H')
   height = 1080
else:
   height = 720

if '-p' in [a[:2] for a in argv[1:]]:
   a = argv[[a[:2] for a in argv[1:]].index('-p') + 1]
   pov_out = a[2:]
   argv.remove(a)

if '-q' in [a[:2] for a in argv[1:]]:
   a = argv[[a[:2] for a in argv[1:]].index('-q') + 1]
   pov_out = a[2:]
   argv.remove(a)
   silent = True

if (h := ('-h' in argv[1:] or '--help' in argv[1:])) or argv[1:] != []:
   if not h:
      stderr.write('Unrecognized: ' + ', '.join(argv[1]) + '\n')
   from os.path import basename
   print (
      'usage ', basename(argv[0]), '[-d]', '[-H]', '[ (-p | -q)<file.png> ]\n'
      '  Writes "out.pov". Outputs povray commandline to stdout.\n'
      '  If color tags are present in inputs, they will be used.\n'
      '  If -d is set, uses decorators.\n'
      '  If -H is set, output is 1080p instead of 720p.\n'
      '  If -n is set, no preview is dispalyed.\n'
      '  If -p is set, calls povray and output the graph.\n'
      '  -q does the same as -p, but quiets povray output.\n'
      '  If these cases, the pov file name is built up from the png file name.'
   )
   exit(0)

from graph_vaux import is_default, color_values, ssys_color, ssys_nebula, ssys_others
from geometry import bb, vec


if pov_out:
   pov = pov_out + '.pov'
else:
   pov = 'out.pov'
dst = open(pov, 'w')

def write_pov( s, indent = -1 ):
   if hasattr(s, '__iter__') and not isinstance(s, str):
      for sub in s:
         write_pov(sub, indent+1)
   elif s.strip() == '':
      dst.write('\n')
   else:
      dst.write(3*indent*' ' + str(s) + '\n')

from graphmod import ssys_pos as V, ssys_jmp as E, no_graph_out
no_graph_out()
colors = { k: color_values[ssys_color(V, k)] for k in V }
nebula = { k: ssys_nebula(V, k) for k in V if ssys_nebula(V, k) is not None}
others = { k: ssys_others(V, k) for k in V }
is_def = { k: is_default((v+['default'])[0]) for k, v in V.aux.items() }

b = bb()
for v in V.values():
   b += v

b *= 1.05
hs = b.size()/2
C = b.mini() + hs
ratio = 1.0 * hs[0] / hs[1]

for i in V:
   if i[0] == '_':
      continue
   V[i] -= C
   V[i] = -V[i]

write_pov([ '',
   '#version 3.7;',
   'global_settings{', [
      'assumed_gamma 1.6',
      'ambient_light 5.0',
   ], '}',
   '',
   'camera {', [
      'orthographic',
      'sky <0,-1,0>',
      'direction <0,1,0>',
      'right ' + str(hs[0]) + '*x',
      'up ' + str(hs[1]) + '*y',
      'location 100*z',
      'look_at 0',
   ], '}',
   '',
])
if show_decorators:
   write_pov([ '#include "decorators.inc"', '', ])

for i, p in V.items():
   if show_decorators and i in decorators:
      nam, depth = decorators[i]
      x, y = str(p[0]), str(p[1])
      write_pov([ 'object{', [
         nam,
         'translate <' + x + ', ' + y + ', ' + str(depth) + '>',
      ], '}', ])
   col = (0.5, 0.5, 0.5) if i not in colors else colors[i]
   if not (i == 'sol'):
      write_pov([ 'sphere{', [
         '<' + str(p[0]) + ', ' + str(p[1]) + ', 0>,',
         '9.0',
         'pigment {color rgb<' + ','.join(map(str, col)) + '>}',
      ], '}', '' ])

   b = None
   if i in nebula:
      q = log((nebula[i]+100.0)/100.0)/log(2.0)
      q = sqrt(q)
      r, g, b = 0.4 + 1.0*q, 0.0, 0.8 - 0.2*q
   elif i in others:
      d = {
         'stellarwind': (0.2, 0.5, 0.7),
         'plasmastorm': (0.5, 0.3, 0.6),
         'haze':        (0.7, 0.0, 0.0)
      }
      for c in (d[t] for t in others[i] if t in d):
         (r, g, b) = c
         break

   if b is not None:
      radius = '7'
      write_pov([ 'cylinder{', [
         '<0,0,-1>,',
         '<0,0,0>,',
         '0.5',
         'pigment {spherical turbulence 0.1 colour_map {[0, rgbt <0,0,0,1>]' +
            '[0.9, rgbt<' + str(r) + ',' + str(g) + ',' + str(b) + ',0.6>]}}',
         'scale 11*' + radius,
         'translate <' + str(p[0]) + ', ' + str(p[1]) + ', 3>',
      ], '}', ''])

   if i == 'sol':
      col = (0.5,  0.0,  1.2)

   if not is_def[i]:
      the_col = tuple([2.0*x for x in col])
      radius = '9' if i == 'sol' else '3'
      write_pov([ 'cylinder{', [
         '<0,0,-1>,',
         '<0,0,0>,',
         '0.7',
         'pigment {spherical turbulence 0.1 colour_map {[0, rgbt <0,0,0,1>]' +
            '[1.0, rgbt<' + ','.join(map(str,the_col)) + ',0.5>]}}',
         'scale 11*' + radius,
         'translate <' + str(p[0]) + ', ' + str(p[1]) + ', 6>',
      ], '}', ''])
   for dstsys, tags in E[i]:
      if dstsys not in V:
         continue
      if p == V[dstsys]:
         stderr.write(i+' and '+dstsys+' have same pos '+str(p)+'!\n')
         continue
      if 'virtual' in tags:
         if not SHOW_VIRTUAL:
            continue
         other = V[dstsys]
      else:
         other = (p + V[dstsys]) / 2.0
      edge_col = '0.3,0.3,0.3'
      tagcol = {'hidden': '0.5,0,0.5', 'virtual': '0,0,1', 'fake': '1,0,0', 'new': '0,1,0' }
      for t in tagcol:
         if t in tags:
            edge_col = tagcol[t]
      write_pov([ 'cylinder{', [
         '<' + str(p[0]) + ', ' + str(p[1]) + ', 0>,',
         '<' + str(other[0]) + ', ' + str(other[1]) + ', 0>,',
         str(3.0 if 'tradelane' in tags else 1.55),
         'pigment {color rgb<' + edge_col + '>}',
      ], '}', '' ])

dst.close()

base = height
cmd = [
   'povray', pov,
   '+W' + str(int(base*ratio)), '+H' + str(base),
   '+A0.1', '+AM2', '+R4', '+BM2'
]
if no_preview:
   cmd += ['-D']

stderr.write(' '.join(cmd) + '\n')

if pov_out is not None:
   if silent:
      arg = {'stderr': open(os.devnull, 'wb')}
   else:
      arg = {}
   subprocess.run(cmd, **arg)
   stderr.write('<' + pov_out + '.png>\n')
