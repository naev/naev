#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import argv, stderr
from graph_vaux import color_values

if argv[1:] != []:
   stderr.write(
      'usage: ' + argv[0].split('/')[-1] + '\n'
      '  Reads a graph file on stdin, outputs a graph on stdout.\n'
      '  Graphmod intended to be used as a preprocessing before neato is applied.\n'
      '  Mainly adds virtual edges.\n'
   )
   exit(0)

anbh = [ 'ngc11935', 'ngc5483', 'ngc7078', 'ngc7533', 'octavian',
   'copernicus', 'ngc13674', 'ngc1562', 'ngc2601', ]

del_edges = [
#   ('titus', 'vedalus'),
#   ('kelvos', 'mason'),
#   ('khaas', 'diadem'),
]

new_edges = [
#  ('khaas', 'vedalus'),
#  ('andres', 'mason'),
]

# In the form: (from, to [, length])
virtual_edges = [
   ('flow', 'basel', 2),
   ('deneb', 'booster', 1.5),
   ('ngc4746', 'logania'),
   #('akodu', 'kenvis'),
   #('tau_ceti', 'sigur'), ('tepvin', 'carrza'),
   ('thirty_stars', 'thorndyke'),
   ('herakin', 'duros'), ('rauthia', 'tide'),
   ('hekaras', 'eneguoz'), ('seifer', 'rei'),
   ('basel', 'octantis'), ('sagittarius', 'baitas'),
   ('baitas', 'tasopa'), ('percival', 'jommel'),
   ('flow', 'katami'), ('nava', 'flow'),
   ('katami', 'eisenhorn'), ('vean', 'basel'),
   ('alpha_centauri', 'tasopa'),('syndania', 'padonia'),
   ('veses', 'protera'), ('syndania', 'stint'),
   ('sagittarius', 'alpha_centauri'), ('protera', 'scholzs_star'),
   ('ngc18451', 'felzen'), ('ngc6057', 'xeric'),
   ('kiwi', 'suna'), ('ngc1098', 'westhaven'),
   ('ngc7061', 'kansas'), ('niger', 'kyo'),
   ('willow', 'palovi'), ('margarita', 'narousse'),
   ('porro', 'modus_manis'), ('suna', 'vanir'),
   ('tobanna', 'brumeria'),('rotide', 'tide'),
   ('padonia', 'basel'), ('ogat', 'wochii'),
   ('griffin', 'pastor'), ('ngc2948', 'ngc9017'),
   ('ngc4131', 'neexi'), ('c59', 'c14'),
   ('c43', 'c28'), ('levo', 'qellan'),
   ('nixon', 'gyrios'), ('suk', 'oxuram'),
   ('defa', 'taiomi'), ('titus', 'solene'), ('titus', 'diadem'),
   ('pike', 'kraft'), ('undergate', 'ulysses'),
   ('ngc20489', 'monogram'), ('anrique', 'adraia'),
   ('andee', 'chraan'), ('trohem', 'tepdania'),
   ('ngc14479', 'zintar'), ('pudas', 'fried'),
   ('blunderbuss', 'darkstone'), ('ekkodu', 'tarsus'),
   ('ivella', 'jommel'), ('starlight_end', 'possum'),
   ('ngc8338', 'unicorn'), ('ngc22375', 'undergate'),
]

prv, prvj  = None, None
for j, i in enumerate(anbh):
   if prv is None:
      prv = i
   else:
      if prvj is not None:
         virtual_edges.append(('_'+str(prvj),    '_'+str(j)))
      prvj = j
      virtual_edges.append(('anubis_black_hole', '_'+str(j)))
      virtual_edges.append(('_'+str(j),                 prv))
      virtual_edges.append(('_'+str(j),                   i))
      prv = None

if prv is not None:
   virtual_edges.append(('_'+str(prvj),   '_'+str(prvj+2)))
   virtual_edges.append(('_'+str(prvj+2),             prv))
   virtual_edges.append(('_'+str(prvj+2),      '_'+str(1)))

from graphmod import ssys_pos, ssys_jmp
from virtual_edges import add_virtual_edges
add_virtual_edges(ssys_jmp, virtual_edges)


for v in ssys_pos:
   for e, t in ssys_jmp[v]:
      if (v, e) in del_edges:
         t.append('fake')
      if (v, e) in new_edges:
         new_edges.remove((v, e))

for (i, j) in new_edges:
   ssys_pos[i].append((j, ['new']))
