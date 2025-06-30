#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, stdin, stderr, exit

from ssys_graph import xml_files_to_graph, default_col, color_values
from geometry import bb, vec


def main( args, pos = None, color = False, halo = False ):
   dst = open('out.pov', 'w')

   def write_pov( s, indent = -1 ):
      if hasattr(s, '__iter__') and not isinstance(s, str):
         for sub in s:
            write_pov(sub, indent+1)
      elif s.strip() == '':
         dst.write('\n')
      else:
         dst.write(3*indent*' ' + str(s) + '\n')

   V, _pos, E, tradelane, colors = xml_files_to_graph(args, color)
   if color:
      colors = { k: color_values[v] for k, v in colors.items() }
   if pos is None or pos == {}:
      pos = _pos
   else:
      V = {k: v for k, v in V.items() if k in pos}

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
            '<' + str(pos[i][0]) + ', ' + str(pos[i][1]) + ', 0>,',
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
            'translate <' + str(pos[i][0]) + ', ' + str(pos[i][1]) + ', 3>',
         ], '}', ''])
      for dstsys, hid in E[i]:
         other = (pos[i] + pos[dstsys]) / 2.0
         write_pov([ 'cylinder{', [
            '<' + str(pos[i][0]) + ', ' + str(pos[i][1]) + ', 0>,',
            '<' + str(other[0]) + ', ' + str(other[1]) + ', 0>,',
            str(2.9 if i in tradelane and dstsys in tradelane else 1.35),
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

if __name__ == '__main__':
   if '-h' in argv[1:] or '--help' in argv[1:]:
      print("usage  ", argv[0], '[-g]', '[-c|-C]', '[<ssys1.xml> ..]')
      print('  Writes "out.pov". Outputs povray commandline to stdout.')
      print('  If -g is set, use the positions from graph read on stdin.')
      print('  If -g is set but no pos in input, uses dat/ssys/*.xml.')
      print('  If -c is set, use faction colors (slower).')
      print('  If -C is set, use color halos (implies -c).')
      print('  Is <ssys.xml> not provided, uses dat/ssys/*.xml.')
      print('examples')
      print('  > $(./utils/ssys/ssys2pov.py -c dat/ssys/*.xml)')
      print('  > display out.png')
   else:
      if input_g := '-g' in argv[1:]:
         argv.remove('-g')

      if color:= '-c' in argv[1:]:
         argv.remove('-c')

      if halo:= '-C' in argv[1:]:
         argv.remove('-C')
         color = True

      if input_g:
         pos = dict()
         for line in stdin:
            if (lin := line.strip()) == '':
               continue
            try:
               nam, x, y = tuple(lin.split(' ')[:3])
               x, y = float(x), float(y)
            except:
               stderr.write('Invalid line in input: "' + lin +'"\n')
               exit(-1)
            pos[nam] = (x, y)
      else:
         pos = None

      args = argv[1:]
      if (ign := [f for f in args if not f.endswith('.xml')]) != []:
         stderr.write('Ignored: "' + '", "'.join(ign) + '"\n')
         args = [f for f in args if f not in ign]
         if args == []:
            stderr.write('No valid input selected. Bye!\n')
            exit(1)

      main(args, pos, color, halo)
