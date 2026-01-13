#!/usr/bin/env python3

import sys
from os import path
dat_dir = path.realpath(path.join(path.dirname(__file__), '..', '..', '..'))
script_dir = path.realpath(path.join(dat_dir, '..', 'utils'))
sys.path.append(script_dir)
sys.path.append(path.realpath(path.join(script_dir, 'outfits')))
from outfit import outfit, naev_xml
from getconst import PHYSICS_SPEED_DAMP

if __name__ != '__main__':
   raise Exception('This is intended to be used as main')

from sys import argv

if len(argv) == 1:
   argv += ['-']

if argv[1] == '-q':
   argv.remove('-q')
   def info(*t):
      pass
else:
   info = lambda *t: sys.stderr.write(' '.join(t) + '\n')

# These are computed by the game and displayed in the race menu
names = ['Peninsula', 'Smiling Man', 'Qex Tour']
length = [23745, 40554, 36886]
#the lengths are overestimated by approx 15%
length = [n / 1.15 for n in length]

configs = [
   {
      'ship': 'neutral/hyena', 'engine': 'small/nexus_dart_160_engine',
      'AB': 'unicorp_light_afterburner',
   },{
      'ship': 'neutral/hyena', 'engine': 'small/nexus_dart_160_engine',
      'AB': 'unicorp_light_afterburner',
   },{
      'ship': 'neutral/hyena', 'engine': 'small/nexus_dart_160_engine',
      'AB': 'unicorp_light_afterburner',
   },{
      'ship': 'soromid/soromid_reaver', 'engines': 'small/tricon_zephyr_engine',
      'accessory': 'racing_trophy_bronze',
      'AG': 'adrenal_gland_ii',
      'skill': 'wanderer'
   },{
      'ship': 'soromid/soromid_reaver', 'engines': 'small/tricon_zephyr_engine',
      'accessory': 'racing_trophy_bronze',
      'AG': 'adrenal_gland_ii',
      'skill': 'wanderer'
   },{
      'ship': 'soromid/soromid_reaver', 'engines': 'small/tricon_zephyr_engine',
      'accessory': 'racing_trophy_bronze',
      'AG': 'adrenal_gland_ii',
      'skill': 'wanderer'
   },{
      'ship': 'pirate/pirate_revenant', 'engine': 'medium/tricon_cyclone_engine',
      'accessory': 'racing_trophy_silver',
      'structure': ('compact_lightsail', 2),
      'AG': 'adrenal_gland_iii',
      'skill': 'wanderer'
   },{
      'ship': 'pirate/pirate_revenant', 'engine': 'medium/tricon_cyclone_engine',
      'accessory': 'racing_trophy_silver',
      'structure': ('compact_lightsail', 2),
      'AG': 'adrenal_gland_iii',
      'skill': 'wanderer'
   },{
      'ship': 'pirate/pirate_revenant', 'engine': 'medium/tricon_cyclone_engine',
      'accessory': 'racing_trophy_silver',
      'structure': ('compact_lightsail', 2),
      'AG': 'adrenal_gland_iii',
      'skill': 'wanderer'
   }
]

ab_ratio = [0.05, 0.198, 0.402, 0.595, 0.602, 0.69, 0.885, 0.89, 0.91]

where = {
   'ship': 'ships/',
   'engine': 'outfits/core_engine/',
   'engines': 'outfits/core_engine/',
   'accessory': 'outfits/accessory/',
   'structure': 'outfits/structure/',
   'AB': 'outfits/utility',
   'AG': 'outfits/bioship/skills/',
   'skill': 'outfits/bioship/skills/',
}


def config_to_str(c):
   s = ''
   for k, t in c.items():
      if not isinstance(t, tuple):
         t = (t, 1 if k != 'engines' else 2)
      s += t[0].split('/')[-1]
      if t[1] != 1:
         s += ' x' + str(t[1])
      s += ' '
   return s

def fmt_tim(f):
   if m := int(f//60):
      return str(m) + "'" + str(round(f-m*60, 1))
   else:
      return str(round(f, 1))

race = [names[i] + ' ' + j for j in ['Bronze', 'Silver', 'Gold'] for i in range(3)]
out={}
for nj, j in enumerate(['Bronze', 'Silver', 'Gold']):
   for ni, i in enumerate(names):
      ra = j + ' ' + i
      l = length[ni]
      ij = nj*3 + ni
      c = configs[ij]
      ab = ab_ratio[ij]
      total = {
         'speed': 0, 'accel': 0,
         'speed_fact': 1.0, 'accel_fact': 1.0,
         'speed_AB': 1.0, 'accel_AB': 1.0, 'action_speed_AB': 1.0}
      for k, v in c.items():
         if isinstance(v,tuple):
            v, count = v
         else:
            count = 1
         fnam = path.join(dat_dir, where[k], v + '.xml')
         if k == 'engine':
            o = outfit(fnam)
            o.stack()
         elif k == 'engines':
            o = outfit(fnam)
            o.stack(o)
         else:
            o = naev_xml(fnam)
         for f in ['speed', 'speed_mod', 'accel', 'accel_mod', 'action_speed']:
            if r := o.find(f):
               r *= count
               if k in {'AB', 'AG'}:
                  total[f + '_AB'] *= 1.0 + (r/100.0)
               elif (w:= f.find('_mod')) != -1:
                  total[f[:w] + '_fact'] *= 1.0 + (r/100.0)
               else:
                  total[f] += r
      for f in ['speed', 'accel']:
         total[f] *= total[f + '_fact']
         del total[f + '_fact']
      in_AB = {k:total[k]*total[k + '_AB'] for k in ['speed', 'accel']}
      final = total['speed'] + total['accel']/PHYSICS_SPEED_DAMP
      final_AB = (in_AB['speed'] + in_AB['accel']/PHYSICS_SPEED_DAMP) * total['action_speed_AB']

      vals = [(a[0].split('/')[-1], a[1]) if isinstance(a, tuple) else a.split('/')[-1] for a in c.values()]
      info('\n' + config_to_str(c))
      s = 'normal: ' + str(round(final))
      s += '  ' + 'AB (\033[36m' + str(round(100.0*ab)) + '%\033[m' + ' of track) ' + str(round(final_AB))
      final = ab*final_AB + (1 - ab)*final
      info(s, '->', '\033[36m' + str(round(final)) + '\033[m')
      info(ra + ':','\033[36;1m' + fmt_tim(l/final) + '\033[m')
      out[i] = out.get(i, {})
      out[i][j] = round(l/final, 1)
info()

with open(argv[1], 'wt') if argv[1] != '-' else sys.stdout as fp:
   fp.write('return {\n')
   for track, times in out.items():
      fp.write(' '*3 + '[' + repr(track) + ']= ')
      fp.write(str(times).replace(':', '=').replace("'",'') + ',\n')
   fp.write('}\n')
