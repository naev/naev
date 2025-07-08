#!/usr/bin/env python3


import os
from sys import stderr, exit, argv, stdout


faction_color = {
   'default':         'default',
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
faction_color[None] = faction_color['default']

faction = {s :s for s in faction_color}
for f in ['wild_ones', 'raven_clan', 'dreamer_clan', 'black_lotus', 'lost', 'marauder']:
   faction[f] = 'pirate'

main_fact = {
   'empire',
   'zalek',
   'dvaered',
   'sirius',
   'soromid',
   'frontier',
   'proteron',
   'thurion',
}
main_col = {faction_color[f] for f in main_fact}

influence_except = {
   'quai', 'dendria', 'surano',  # pirate blockades
   'gamel', 'acheron', 'muran', 'sigur', 'ngc10284', 'sheffield', # pirate access
}

if __name__ != '__main__':
   color_values = {
      'default': (0.25, 0.25, 0.25),
      'green':   (0.0,  0.75,  0.0),
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
   is_default = lambda s: s is None or s.split('@')[0] == 'default'
   col_avg = lambda u, v: tuple([(i + 1.5*j + 0.5*0) / 4.0 for (i, j) in zip(u, v)])
   for c in main_col:
      color_values['default@' + c] = col_avg(default_col, color_values[c])

   def _ssys_color( V, ssys ):
      return (V.aux[ssys] + ['default'])[0]

   def ssys_color( V, ssys ):
      return _ssys_color(V, ssys).split(':', 1)[0]

   def ssys_nebula( V, ssys ):
      return _ssys_color(V, ssys).split(':', 1)[1:2] == [ 'nebula' ]

else:
   if do_color := ('-c' in argv[1:]):
      argv.remove('-c')

   if extended := ('-e' in argv[1:]):
      argv.remove('-e')

   if do_names := ('-n' in argv[1:]):
      argv.remove('-n')

   if (help_f := '-h' in argv or '--help' in argv[1:]) or argv[1:] != []:
      msg = lambda s: (stdout if help_f else stderr).write(s + '\n')
      DOC = [
         'usage:  ' + os.path.basename(argv[0]) + '[ -c ] [ -n ] [-e]',
         '  Adds faction name to vertices aux field.',
         '  If -c is set, adds color instead.',
         '  If -c and -n is set, adds color + name.',
         '  If only -n is set, adds name.',
         '  If -e is set, extends faction/color tags to influence zone.',
      ]
      for l in DOC:
         msg(l)
      exit(0 if help_f else 1)

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

      if not(do_names and not do_color and not extended):
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
         aux = 'default' if aux is None else aux
         if aux == 'default':
            if do_names or extended:
               V.aux[bnam].append('default')
         else:
            V.aux[bnam].append(aux)
         if V.aux[bnam] != [] and T.find('general/nebula') is not None:
            V.aux[bnam][-1] += ':nebula'
         if do_names:
            V.aux[bnam].extend(T.attrib['name'].split(' '))
   if extended:
      from graphmod import sys_jmp as E
      newt = {}
      infl = main_col if do_color else main_fact
      _is_def = lambda a: a == [] or a[0] == 'default'
      _nhn = lambda i: [j for j, k in E[i] if 'hidden' not in k]
      for bnam in V:
         if _is_def(V.aux[bnam]) and bnam not in influence_except:
            newt[bnam] = None
            N = [(j, V.aux[j]) for j in _nhn(bnam) if bnam in _nhn(j)]
            facts = [(e, a[0]) for (e, a) in N if not _is_def(a)]
            if len({j for i, j in facts}) == 1 and (f := facts[0][1]) in infl:
               newt[bnam] = f
         elif V.aux[bnam][0].split(':')[-1] == 'nebula':
            newt[bnam] = 'nebula'
         else:
            newt[bnam] = V.aux[bnam][0]

      twn = lambda i: [j for j in _nhn(i) if newt[j] != 'nebula' and i in _nhn(j)]
      def unpropag( i, val ):
         if newt[i] in [None, True]:
            newt[i] = val
            for k in twn(i):
               unpropag(k, val)

      def propag( i, crt ):
         out = newt[i]
         if newt[i] is None:
            newt[i] = True
            for k in twn(i):
               res = propag(k, crt)
               if res not in crt:
                  if len(crt) == 1:
                     crt.append(res)
                  else:
                     return 'default'
               out = crt[-1]
         return out

      for bnam in V:
         if newt[bnam] is None:
            res = propag(bnam, [True])
            unpropag(bnam, res if res in infl else 'default')

         if newt[bnam] != 'default':
            if V.aux[bnam] == []:
               V.aux[bnam].append('default')
            if V.aux[bnam][0] == 'default':
               V.aux[bnam][0] += '@' + newt[bnam]

