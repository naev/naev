#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, stderr

from ssys_graph import xml_files_to_graph, default_col
from geometry import bb, vec


def main( args, color = False, halo = False ):
   dst = open('out.pov', 'w')
   V, pos, E, tradelane, colors = xml_files_to_graph(args, color)
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
   #version 3.7;
   global_settings{
      assumed_gamma 1.6
      ambient_light 5.0
   }

   camera {
      orthographic
      sky <0,-1,0>
      direction <0,1,0>\n""")
   dst.write(6*' ' + 'right ' + str(hs[0]) + '*x\n')
   dst.write(6*' ' + 'up ' + str(hs[1]) + '*y\n')
   dst.write("""      location 100*z
      look_at 0
   }\n""")

   for i in V:
      if i not in colors:
         col = (0.5,0.5,0.5)
      else:
         col = colors[i]
      dst.write(3*' ' + 'sphere{\n')
      dst.write(6*' ' + '<' + str(pos[i][0]) + ', ' + str(pos[i][1]) + ', 0>,\n')
      dst.write(6*' ' + '9.0\n')
      dst.write(6*' ' + 'pigment {color rgb<' + ','.join(map(str,col)) + '>}\n')
      dst.write(3*' ' + '}\n')

      if halo and col != default_col:
         dst.write(3*' ' + 'cylinder{\n')
         dst.write(6*' ' + '<0,0,-1>,\n')
         dst.write(6*' ' + '<0,0,0>,\n')
         dst.write(6*' ' + '0.7\n')
         dst.write(6*' ' + 'pigment {spherical turbulence 0.1 colour_map {[0, rgbt <0,0,0,1>][1.0, rgbt<')
         dst.write(','.join(map(str,col)) + ',0>]}}\n')
         dst.write(6*' ' + 'scale 11*3\n')
         dst.write(6*' ' + 'translate <' + str(pos[i][0]) + ', ' + str(pos[i][1]) + ', 3>\n')
         dst.write(3*' ' + '}\n')
      for dstsys, hid in E[i]:
         dst.write(3*' ' + 'cylinder{\n')
         dst.write(6*' ' + '<' + str(pos[i][0]) + ', ' + str(pos[i][1]) + ', 0>,\n')
         other = (pos[i] + pos[dstsys]) / 2.0
         dst.write(6*' ' + '<' + str(other[0]) + ', ' + str(other[1]) + ', 0>,\n')
         if i in tradelane and dstsys in tradelane:
            size = 2.9
         else:
            size = 1.35
         dst.write(6*' ' + str(size) + '\n')

         if hid:
            dst.write(6*' ' + 'pigment {color rgb<0.5,0,0>}\n')
         else:
            dst.write(6*' ' + 'pigment {color rgb<0.3,0.3,0.3>}\n')
         dst.write(3*' ' + '}\n')
   dst.close()
   cmd = ['povray','out.pov']
   base = 1080
   cmd += ['+W' + str(base*ratio), '+H' + str(int(base)), '+A0.1', '+AM2', '+R4', '+BM2']
   print(' '.join(cmd))

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:] or len(argv) < 2:
      print("usage  ", argv[0], '[-c|-C]', '<ssys1.xml> ...')
      print('  Writes "out.pov". Outputs povray commandline to stdout.')
      print('  If -c is set, use faction colors (slower).')
      print('  If -C is set, use color halos (implies -c).')
      print('examples')
      print('  > $(./utils/ssys/ssys2pov.py -c dat/ssys/*.xml)')
      print('  > display out.png')
   else:
      if color:= '-c' in argv[1:]:
         argv.remove('-c')

      if halo:= '-C' in argv[1:]:
         argv.remove('-C')
         color = True

      if (ign := [f for f in argv[1:] if not f.endswith('.xml')]) != []:
         stderr.write('Ignored: "' + '", "'.join(ign) + '"\n')

      main([f for f in argv[1:] if f.endswith('.xml')], color, halo)
