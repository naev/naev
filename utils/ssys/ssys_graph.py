# python3


import os
from sys import stderr, exit
import xml.etree.ElementTree as ET
from ssys import nam2base, getpath, PATH


twins = {
   ('carnis_minor', 'carnis_major'): 0.2,
   ('gliese', 'gliese_minor'): 0.5,
   ('kruger', 'krugers_pocket'): 0.1
}

twins = {tuple(sorted(k)):v for k, v in twins.items()}

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

for f in ['wild_ones', 'raven_clan', 'dreamer_clan', 'black_lotus', 'lost', 'marauder']:
   faction_color[f] = faction_color['pirate']

def get_spob_faction( nam ):
   T = ET.parse(getpath(PATH, "spob", nam + ".xml")).getroot()
   e = T.find("./presence/faction")
   return None if e is None else nam2base(e.text)

def all_ssys( args = None ):
   def gen():
      if args is None or args == []:
         path = os.path.join(PATH, 'ssys')
         for arg in os.listdir(path):
            yield arg, os.path.join(path, arg)
      else:
         for a in args:
            yield os.path.basename(a), a
   for a, b in gen():
      if a[-4:] == '.xml':
         yield a[:-4], b

# Vnames, Vpos, E, tradelane, color = xml_files_to_graph(args, use_color)
def xml_files_to_graph( args = None, get_colors = False ):
   name2id = dict()
   name, acc, pos, tradelane, color = [], [], [], set(), dict()
   for bname, filename in all_ssys(args):
      name.append(bname)
      T=ET.parse(filename).getroot()

      try:
         name[-1] = T.attrib['name']
      except:
         stderr.write('no name defined in "' + bname + '"\n')

      try:
         e = T.find('pos')
         pos.append((bname, (e.attrib['x'], e.attrib['y'])))
      except:
         stderr.write('no position defined in "' + bname + '"\n')

      for e in T.findall('tags/tag'):
         if e.text == 'tradelane':
            tradelane.add(bname)
            break

      if get_colors:
         fact = dict()
         spobs = []
         for e in T.findall('spobs/spob'):
            spobs.append(nam2base(e.text))

         nb = len(spobs)
         maxi = 0

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
         color[bname] = faction_color[fact]

      name2id[name[-1]] = bname
      acc.append([])
      count = 1
      for e in T.findall('./jumps/jump'):
         try:
            acc[-1].append((e.attrib['target'], e.find('hidden') is not None))
         except:
            stderr.write('no target defined in "'+filename+'"jump#'+str(count)+'\n')
      count += 1

   ids = [name2id[x] for x in name]
   acc = [[(name2id[t[0]],t[1]) for t in L] for L in acc]
   return dict(zip(ids,name)), dict(pos), dict(zip(ids,acc)), tradelane, color

