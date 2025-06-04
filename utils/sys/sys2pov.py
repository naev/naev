#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, stderr

from sys2graph import xml_files_to_graph
from geometry import bb, vec


def main( args ):
   dst = open('out.pov', 'w')
   V, E, pos, tradelane = xml_files_to_graph(args)
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
      dst.write(6*' ' + 'pigment {color rgb<1,1,1>}\n')
      dst.write(3*' ' + '}\n')
      for dstsys, hid in E[i]:
         dst.write(3*' ' + 'cylinder{\n')
         dst.write(6*' ' + '<' + str(pos[i][0]) + ', ' + str(pos[i][1]) + ', 0>,\n')
         other = (pos[i] + pos[dstsys]) / 2.0
         dst.write(6*' ' + '<' + str(other[0]) + ', ' + str(other[1]) + ', 0>,\n')
         if i in tradelane and dstsys in tradelane:
            size = 2.0
         else:
            size = 1.0
         dst.write(6*' ' + str(size) + '\n')

         if hid:
            dst.write(6*' ' + 'pigment {color rgb<1,0,0>}\n')
         else:
            dst.write(6*' ' + 'pigment {color rgb<1,1,1>}\n')
         dst.write(3*' ' + '}\n')
   dst.close()
   cmd = ['povray','out.pov']
   base = int(1280*1.0)
   cmd += ['+W' + str(base), '+H' + str(int(base/ratio)), '+A0.2', '+AM2', '+J']
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
