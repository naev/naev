#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, stdin, stderr, exit
import subprocess
import os

from graph_vaux import is_default, color_values, ssys_color, ssys_nebula
from geometry import bb, vec


def main( pov_out = None, halo = False, silent = False ):
   if pov_out:
      pov = pov_out.replace('.png', '.pov', 1)
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

   from graphmod import sys_pos as V, sys_jmp as E
   E.silence()
   V.silence()
   colors = { k: color_values[ssys_color(V, k)] for k in V }
   nebula = { k for k in V if ssys_nebula(V, k) }
   is_def = { k: is_default((v+['default'])[0]) for k, v in V.aux.items() }

   b = bb()
   for i in V:
      b += V[i]

   b *= 1.05
   hs = b.size()/2
   C = b.mini() + hs
   ratio = 1.0 * hs[0] / hs[1]

   for i in V:
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
   for i in V:
      col = (0.5, 0.5, 0.5) if i not in colors else colors[i]
      if not (i == 'sol' and halo):
         write_pov([ 'sphere{', [
            '<' + str(V[i][0]) + ', ' + str(V[i][1]) + ', 0>,',
            '9.0',
            'pigment {color rgb<' + ','.join(map(str, col)) + '>}',
         ], '}', '' ])

      if i == 'sol':
         col = (0.5,  0.0,  1.2)

      if i in nebula:
         radius = '7'
         write_pov([ 'cylinder{', [
            '<0,0,-1>,',
            '<0,0,0>,',
            '0.5',
            'pigment {spherical turbulence 0.1 colour_map {[0, rgbt <0,0,0,1>]' +
               '[0.9, rgbt<0.4,0,0.8,0.5>]}}',
            'scale 11*' + radius,
            'translate <' + str(V[i][0]) + ', ' + str(V[i][1]) + ', 3>',
         ], '}', ''])
      if halo and not is_def[i]:
         the_col = tuple([2.0*x for x in col])
         radius = '5' if i == 'sol' else '3'
         write_pov([ 'cylinder{', [
            '<0,0,-1>,',
            '<0,0,0>,',
            '0.7',
            'pigment {spherical turbulence 0.1 colour_map {[0, rgbt <0,0,0,1>]' +
               '[1.0, rgbt<' + ','.join(map(str,the_col)) + ',0.5>]}}',
            'scale 11*' + radius,
            'translate <' + str(V[i][0]) + ', ' + str(V[i][1]) + ', 6>',
         ], '}', ''])
      for dstsys, tags in E[i]:
         hid = 'hidden' in tags
         other = (V[i] + V[dstsys]) / 2.0
         write_pov([ 'cylinder{', [
            '<' + str(V[i][0]) + ', ' + str(V[i][1]) + ', 0>,',
            '<' + str(other[0]) + ', ' + str(other[1]) + ', 0>,',
            str(2.9 if 'tradelane' in tags else 1.4),
            'pigment {color rgb<' + ('0.5,0,0' if hid else '0.3,0.3,0.3') + '>}',
         ], '}', '' ])

   dst.close()

   base = 1080
   cmd = [
      'povray', pov,
      '+W' + str(int(base*ratio)), '+H' + str(base),
      '+A0.1', '+AM2', '+R4', '+BM2'
   ]
   if pov_out is not None:
      cmd += ['+O' + pov_out]

   stderr.write(' '.join(cmd) + '\n')

   if pov_out is not None:
      if silent:
         arg = {'stderr':open(os.devnull, 'wb')}
      else:
         arg = {}
      subprocess.run(cmd, **arg)
      stderr.write('<' + pov_out + '>\n')

non_opt = [a for a in argv[1:] if a[:2] not in ['-c', '-p', '-q']]
if '-h' in non_opt or '--help' in non_opt or non_opt!=[]:
   print("usage  ", argv[0], '[-c] [ (-p | -q)<file.png> ]')
   print('  Writes "out.pov". Outputs povray commandline to stdout.')
   print('  If color tags are present in inputs, they will be used.')
   print('  If -c is set, use color halos.')
   print('  If -p is set, calls povray and output the graph.')
   print('  -q does the same as -p, but quiets povray output.')
   print('  If these cases, the pov file name is built up from the png file name.')
else:
   silent = False
   if halo:= '-s' in argv[1:]:
      argv.remove('-s')

   if halo:= '-c' in argv[1:]:
      argv.remove('-c')

   pov_out = None
   if '-p' in [a[:2] for a in argv[1:]]:
      a = argv[[a[:2] for a in argv[1:]].index('-p') + 1]
      pov_out = a[2:]
      argv.remove(a)

   if '-q' in [a[:2] for a in argv[1:]]:
      a = argv[[a[:2] for a in argv[1:]].index('-q') + 1]
      pov_out = a[2:]
      argv.remove(a)
      silent = True

   main(pov_out, halo, silent)
