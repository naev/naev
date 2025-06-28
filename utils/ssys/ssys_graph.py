#!/usr/bin/env python3


import os
from sys import stderr, exit
import xml.etree.ElementTree as ET
from ssys import nam2base, getpath, PATH


twins = {
   ('carnis_minor', 'carnis_major'): 0.7,
   ('gliese', 'gliese_minor'): 0.5,
   ('kruger', 'krugers_pocket'): 0.5
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


if __name__ == '__main__':
   from sys import argv, exit, stdout, stderr, stdin

   def do_reading( args, do_V = True, do_E = True ):
      _Vnames, Vpos, E, tradelane, color = xml_files_to_graph(args, do_V)
      if do_V:
         for bname, (x, y) in Vpos.items():
            print(bname, x, y, color[bname])
      if do_E:
         for fst, L in E.items():
            for snd, hid in L:
               if hid:
                  print(fst, snd, 'hidden')
               elif fst in tradelane and snd in tradelane:
                  print(fst, snd, 'tradelane')
               elif (t:= tuple(sorted([fst, snd]))) in twins:
                  print(fst, snd, twins[t])
               else:
                  print(fst, snd)

   def _read_stdin_and_scale( scale ):
      for l in stdin:
         if (line := l.strip()) != '':
            try:
               (n, x, y, r) = (l.strip().split(' ',4) + ['', '']) [:4]
               x = str(float(x) * scale)
               y = str(float(y) * scale)
               yield n, x, y, r
            except:
               pass

   def do_writing( scale = 1.0 ):
      from ssys import fil_ET
      for n, x, y, _ in _read_stdin_and_scale(scale):
         name = os.path.join(PATH, 'ssys', n + '.xml')
         T = fil_ET(name)
         e = T.getroot().find('pos')
         e.attrib['x'], e.attrib['y'] = x, y
         T.write(name)

   def do_scaling( scale = 1.0 ):
      for t in _read_stdin_and_scale(scale):
         print(' '.join(t).strip())

   if only_V:= ('-v' in argv[1:]):
      argv.remove('-v')

   if only_E:= ('-e' in argv[1:]):
      argv.remove('-e')

   if do_write := ('-w' in argv[1:]):
      argv.remove('-w')
      scale = 1.0

   if do_scale := ('-s' in argv[1:]):
      index = argv.index('-s')
      argv.pop(index)
      try:
         scale = float(argv.pop(index))
      except:
         stderr.write('Error: expected <scale> after -s.\n')
         exit(1)

   help_f = '-h' in argv or '--help' in argv[1:]
   if help_f or (argv[1:] != [] and do_write):
      msg = lambda s: (stdout if help_f else stderr).write(s + '\n')
      DOC = [
         'usage:  ' + os.path.basename(argv[0]) +
            '[ -v | -e ]  (-s <scale> [-w]) | -w | [<files>..]',
         '  Output the (enriched) graph of ssys',
         '  Vertices are in the form:',
         '  <sys_name> <x> <y> <color>',
         '  Edges are in the form:',
         '  <src_sys_name> <dst_sys_name> <aux>',
         '  If <files.xml> is provided, only outputs vertices provided.',
         '  If -v is set, only outputs vertices.',
         '  If -e is set, only outputs edges.',
         '',
         '  If -s is set, reads (ssys, x, y, ...) on stdin, rescales it',
         '  with <scale> and, unless -w is set, outputs the result on stdout.',
         '  If -w is set, reads (ssys, x, y, ...) on stdin and update dat/ssys.',
      ]
      for l in DOC:
         msg(l)
      exit(0 if ok else 1)

   if do_write:
      do_writing(scale)
   elif do_scale:
      do_scaling(scale)
   else:
      do_reading(argv[1:], not only_E, not only_V)
