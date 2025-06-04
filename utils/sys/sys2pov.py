#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, stderr

from sys2graph import xml_files_to_graph
from geometry import bb, vec


faction_color = {
   None : '<0.3,0.3,0.3>',
   'empire':'<0,1,0>',
   'zalek':'<0.5,0,0>',
   'dvaered':'<0.5,0.2,0>',
   'sirius':'<0.0,0.7,0.8>',
   'soromid':'<1.0,0.5,0.0>',
   'frontier':'<1.0,1.0,0.0>',
   'pirate':'<1.0,0.0,0.0>',
   'independant' : '<0.0,0.0,1.0>',
   'proteron' : '<1.0,0.0,1.0>',
   'thurion' : '<0.5,0.5,0.5>',
   'collective' : '<0.9,0.9,0.9>',
   'goddard' : '<0.0,0.1,1.0>',
   'traders_society' : '<0.0,0.1,1.0>',
   'orez' : '<0.0,0.4,0.9>',
}

for f in ['wild_ones', 'raven_clan', 'dreamer_clan', 'black_lotus', 'lost']:
   faction_color[f] = faction_color['pirate']

def main( args ):
   dst = open('out.pov', 'w')
   V, E, pos, tradelane, faction = xml_files_to_graph(args)
   b = bb()
   
   for i in V:
      pos[i] = vec(pos[i])
      b += pos[i]

   b *= 1.05
   hs = b.size()/2
   C = b.mini() + hs
   ratio = 1.0 * hs[0] / hs[1]

   for i in V:
      pos[i] -= C
      pos[i] = -pos[i]

   dst.write("""
   global_settings{
      assumed_gamma 1.6
      ambient_light 5.0
   }

   camera {
      orthographic
      sky <0,0,1>
      direction <0,1,0>\n""")
   dst.write(6*' ' + 'right ' + str(hs[0]) + '*x')
   dst.write(6*' ' + 'up ' + str(hs[1]) + '*y')
   dst.write("""      location 100*z
      look_at 0
   }\n""")

   for i in V:
      dst.write(3*' ' + 'sphere{\n')
      dst.write(6*' ' + '<' + str(pos[i][0]) + ', ' + str(pos[i][1]) + ', 0>,\n')
      dst.write(6*' ' + '10.0\n')
      if i not in faction:
         faction[i] = None
      if faction[i] not in faction_color:
         faction_color[faction[i]] = faction_color[None]
      dst.write(6*' ' + 'pigment {color rgb' + faction_color[faction[i]] + '}\n')
      dst.write(3*' ' + '}\n')
      for dstsys, hid in E[i]:
         dst.write(3*' ' + 'cylinder{\n')
         dst.write(6*' ' + '<' + str(pos[i][0]) + ', ' + str(pos[i][1]) + ', 0>,\n')
         other = (pos[i] + pos[dstsys]) / 2.0
         dst.write(6*' ' + '<' + str(other[0]) + ', ' + str(other[1]) + ', 0>,\n')
         if i in tradelane and dstsys in tradelane:
            size = 2.5
         else:
            size = 1.2
         dst.write(6*' ' + str(size) + '\n')

         if hid:
            dst.write(6*' ' + 'pigment {color rgb<0.8,0,0>}\n')
         else:
            dst.write(6*' ' + 'pigment {color rgb<0.5,0.5,0.5>}\n')
         dst.write(3*' ' + '}\n')
   dst.close()
   cmd = ['povray','out.pov']
   base = 1080
   cmd += ['+W' + str(base*ratio), '+H' + str(int(base)), '+A0.2', '+AM2', '+J', '+BM2']
   print(' '.join(cmd))

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv)<2:
      print("usage:",argv[0],'<sys1.xml> ...')
      print('Writes "out.pov". Outputs povray commandline to stdout.')
      print('Examples:')
      print('  > $(./utils/sys2pov.py dat/ssys/*.xml)')
      print('  > display out.png')
   else:
      if (ign := [f for f in argv[1:] if not f.endswith('.xml')]) != []:
         stderr.write('Ignored: "' + '", "'.join(ign) + '"\n')

      main([f for f in argv[1:] if f.endswith('.xml')]) 
