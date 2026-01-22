#!/usr/bin/env python3

configs = [
   {
      'ship':     'neutral/hyena',
      'engine':   'small/unicorp_hawk_160_engine',
      'system':   'small/unicorp_pt16_core_system',
      'hull':     'small/unicorp_d2_light_plating'
   }, {
      'ship':     'neutral/lancelot',
      'engines':  'small/unicorp_hawk_160_engine',
      'systems':  'small/unicorp_pt16_core_system',
      'hulls':    'small/unicorp_d2_light_plating'
   }, {
      'ship':     'neutral/admonisher',
      'engine':   'medium/unicorp_falcon_700_engine',
      'system':   'medium/unicorp_pt200_core_system',
      'hull':     'medium/unicorp_d23_medium_plating'
   }, {
      'ship':     'neutral/pacifier',
      'engines':  'medium/unicorp_falcon_700_engine',
      'systems':  'medium/unicorp_pt200_core_system',
      'hulls':    'medium/unicorp_d23_medium_plating'
   }, {
      'ship':     'neutral/kestrel',
      'engine':   'large/unicorp_eagle_3000_engine',
      'system':   'large/unicorp_pt440_core_system',
      'hull':     'large/unicorp_d58_heavy_plating'
   }, {
      'ship':     'neutral/goddard',
      'engines':  'large/unicorp_eagle_3000_engine',
      'systems':  'large/unicorp_pt440_core_system',
      'hulls':    'large/unicorp_d58_heavy_plating'
   }
]

import sys
from os import path
from sys import argv
script_dir = path.realpath(path.join(path.dirname(__file__), '..'))
dat_dir = path.realpath(path.join(script_dir, '..'))

sys.path.append(script_dir)
sys.path.append(path.realpath(path.join(script_dir, 'outfits')))

from outfit import outfit, naev_xml
from getconst import PHYSICS_SPEED_DAMP

if __name__ != '__main__':
   raise Exception('This is intended to be used as main')

if '-q' in argv[1:]:
   argv.remove('-q')
   def info(*t):
      pass
else:
   info = lambda *t: sys.stderr.write(' '.join(t) + '\n')

if len(argv) == 1:
   argv += ['-']

where = {
   'ship':   'ships/',
   'system': 'outfits/core_system/',
   'engine': 'outfits/core_engine/',
   'hull':   'outfits/core_hull/',
}

where |= { k+'s': where[k] for k in where }

from math import sqrt

for n,c in enumerate(configs):
   armor, mass = 0, 0
   for k, v in c.items():
      fnam = path.join(dat_dir, where[k], v + '.xml')
      if k in ['engine', 'system', 'hull']:
         o = outfit(fnam)
         o.stack()
      elif k in ['engines', 'systems', 'hulls']:
         o = outfit(fnam)
         o.stack(o)
      else:
         o = naev_xml(fnam)
      d = o.to_dict()
      m, a=mass, armor
      mass += d.get('mass', 0)
      armor+= d.get('armour', 0)
   print (str(n+1) + ' ' + str(armor/(0.3*sqrt(mass))) + ' ' + str(armor) + ' ' + str(mass))
