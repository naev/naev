#!/usr/bin/env python3


import os
from sys import stderr, exit, argv


faction_color = {
   None:              'default',
   'empire':          'green',
   'zalek':           'darkred',
   'dvaered':         'brown',
   'sirius':          'teal',
   'soromid':         'orange',
   'frontier':        'yellow',
   'pirate':          'red',
   'independent':     'blue',
   'proteron':        'magenta',
   'thurion':         'grey',
   'collective':      'white',
   'goddard':         'blue2',
   'traders_society': 'blue2',
   'orez':            'skyblue',
   'flf':             'yellow',
}


if __name__ != '__main__':
   color_values = {
      'default': (0.25, 0.25, 0.25),
      'green':   (0.0,  0.85, 0.0),
      'darkred': (0.6,  0.0,  0.0),
      'brown':   (0.6,  0.2,  0.0),
      'teal':    (0.0,  0.7,  0.8),
      'orange':  (0.95, 0.5,  0.0),
      'yellow':  (0.8,  0.8,  0.0),
      'red':     (1.0,  0.0,  0.0),
      'blue':    (0.0,  0.0,  1.0),
      'magenta': (0.9,  0.0,  0.9),
      'grey':    (0.5,  0.5,  0.5),
      'white':   (0.8,  0.8,  0.8),
      'blue2':   (0.0,  0.1,  1.0),
      'skyblue': (0.0,  0.4,  0.9),
   }

   default_col = color_values['default']
else:
   if do_color := ('-c' in argv[1:]):
      argv.remove('-c')

   if (help_f := '-h' in argv or '--help' in argv[1:]) or argv[1:] != []:
      msg = lambda s: (stdout if help_f else stderr).write(s + '\n')
      DOC = [
         'usage:  ' + os.path.basename(argv[0]) + ' [ -c ]',
         '  Adds faction name to vertices aux field.',
         '  If -c is set, adds color instead.',
      ]
      for l in DOC:
         msg(l)
      exit(0 if ok else 1)


   faction = {s :s for s in faction_color}

   for f in ['wild_ones', 'raven_clan', 'dreamer_clan', 'black_lotus', 'lost', 'marauder']:
      faction[f] = 'pirate'


   import xml.etree.ElementTree as ET
   from ssys import getpath, PATH, nam2base

   def get_spob_faction( nam ):
      T = ET.parse(getpath(PATH, "spob", nam + ".xml")).getroot()
      e = T.find("./presence/faction")
      if e is None:
         return None
      fnam = nam2base(e.text)
      return None if fnam not in faction else faction[fnam]


   from graphmod import sys_pos as V

   for bnam in V:
      T = ET.parse(getpath(PATH, "ssys", bnam + ".xml")).getroot()

      spobs = []
      for e in T.findall('spobs/spob'):
         spobs.append(nam2base(e.text))

      nb = len(spobs)
      maxi = 0
      fact = {}
      for s in spobs:
         res = None
         if f := get_spob_faction(s):
            if f not in fact:
               fact[f] = 0
            res = f;
         if res is None :
            nb -= 1
         else:
            fact[f] += 1
            if fact[f] > maxi:
               maxi = fact[f]
         if maxi >= (nb+1) / 2:
            break

      fact = list(sorted([(n,f) for (f,n) in fact.items()], reverse = True))
      if len(fact)>1 and fact[0][1]=='independent' and fact[0][0] == fact[1][0]:
         fact = fact[1]
      else:
         fact = (fact+[(None,None)])[0]
      if (fact:= fact[1]) not in faction_color:
         fact = None

      aux = faction_color[fact] if do_color else fact
      if aux is not None:
         V.aux[bnam].append(aux)
