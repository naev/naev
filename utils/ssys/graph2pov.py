#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, stdin, stderr, exit

from ssys_graph import xml_files_to_graph, default_col, color_values
from geometry import bb, vec


def main( halo = False ):
   dst = open('out.pov', 'w')

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
   colors = { k: color_values[v[0]] for k, v in V.aux.items()}

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
      col = (0.5,0.5,0.5) if i not in colors else colors[i]
      if not (i == 'sol' and halo):
         write_pov([ 'sphere{', [
            '<' + str(V[i][0]) + ', ' + str(V[i][1]) + ', 0>,',
            '9.0',
            'pigment {color rgb<' + ','.join(map(str, col)) + '>}',
         ], '}', '' ])

      if i == 'sol':
         col = (0.3,  0.0,  1.2)

      if halo and col != default_col:
         radius = '9' if i == 'sol' else '3'
         write_pov([ 'cylinder{', [
            '<0,0,-1>,',
            '<0,0,0>,',
            '0.7',
            'pigment {spherical turbulence 0.1 colour_map {[0, rgbt <0,0,0,1>]' +
               '[1.0, rgbt<' + ','.join(map(str,col)) + ',0>]}}',
            'scale 11*' + radius,
            'translate <' + str(V[i][0]) + ', ' + str(V[i][1]) + ', 3>',
         ], '}', ''])
      for dstsys, tags in E[i]:
         hid = 'hidden' in tags
         other = (V[i] + V[dstsys]) / 2.0
         write_pov([ 'cylinder{', [
            '<' + str(V[i][0]) + ', ' + str(V[i][1]) + ', 0>,',
            '<' + str(other[0]) + ', ' + str(other[1]) + ', 0>,',
            str(2.9 if 'tradelane' in tags else 1.35),
            'pigment {color rgb<' + ('0.5,0,0' if hid else '0.3,0.3,0.3') + '>}',
         ], '}', '' ])

   dst.close()

   base = 1080
   cmd = [
      'povray', 'out.pov',
      '+W' + str(int(base*ratio)), '+H' + str(base),
      '+A0.1', '+AM2', '+R4', '+BM2'
   ]
   print(' '.join(cmd))

if '-h' in argv[1:] or '--help' in argv[1:] or argv[1:]!=['-C']:
   print("usage  ", argv[0], '[-C]')
   print('  Writes "out.pov". Outputs povray commandline to stdout.')
   print('  If color tags are present in inputs, they will be used.')
   print('  If -C is set, use color halos.')
else:
   if halo:= '-C' in argv[1:]:
      argv.remove('-C')

   main(halo)
