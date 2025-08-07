#!/usr/bin/env python3


import os
from sys import stderr, exit
import xml.etree.ElementTree as ET
from ssys import nam2base, getpath, PATH, naev_xml
from graph_vaux import ssys_others, ssys_nam



if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

from sys import argv, exit, stdout, stderr, stdin

help_f = '-h' in argv or '--help' in argv[1:]
if help_f or (argv[1:] and do_write):
   msg = lambda s: (stdout if help_f else stderr).write(s + '\n')
   DOC = [
      'usage:  ' + os.path.basename(argv[0]),
      '  Updates ssys/*.xml according to the graph provided in input.'
   ]
   for l in DOC:
      msg(l)
   exit(0 if help_f else 1)

def new_ssys(name, basenam, pos, ssys_pos, jmp):
   xml = naev_xml()
   xml.save_as(name)
   Nam = ssys_nam(ssys_pos, basenam)
   if Nam and nam2base(Nam) != basenam:
      stderr.write('Warning: basename "' + basenam + '" does not match provided name "' + Nam  + '"\n')
   mk_jump = lambda dst, aux: {
      '@target': ssys_nam(ssys_pos, dst),
      'autopos': {},
      'hide': 1,
      'tags': {'tag': aux},
   }
   xml['ssys'] = {
      '@name': Nam or ' '.join([s[0].upper() + s[1:] for s in basenam.split('_')]),
      'general': {'radius': 5000, 'spacedust': 300, 'interference': 0},
      'pos': {'@x': pos[0], '@y': pos[1]},
      'spobs': {},
      'jumps': {'jump': [mk_jump(k, v) for k, v in jmp.items()]},
      'asteroids': {},
      'tags': {'tag': list(set(ssys_others(ssys_pos, basenam)) - {'new', 'update'}) },
   }
   if 'stellarwind' in ssys_others(ssys_pos, basenam):
      xml['ssys']['general'] |= {
        'background': 'stellarwind',
        'map_shader': 'stellarwind_map.frag',
        'features': '#bStellar Wind (1 fuel regen)#0'
      }
   xml.save()

from graphmod import ssys_pos, ssys_jmp, no_graph_out

no_graph_out()

from ssys import fil_ET
for n, (x, y) in ssys_pos.items():
   name = os.path.join(PATH, 'ssys', n + '.xml')
   try:
      T = fil_ET(name)
      e = T.getroot().find('pos')
      e.attrib['x'], e.attrib['y'] = str(x), str(y)
      T.write(name)
   except FileNotFoundError:
      new_ssys(name, n, (x, y), ssys_pos, ssys_jmp[n])
