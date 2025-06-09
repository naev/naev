#!/usr/bin/env python3


from sys import stderr
import xml.etree.ElementTree as ET
from os.path import basename
from ssys import nam2base, getpath, PATH


default_col = (0.25, 0.25, 0.25)
faction_color = {
   None:                default_col,
   'empire':            (0.0,  0.85, 0.0),
   'zalek':             (0.6,  0.0,  0.0),
   'dvaered':           (0.6,  0.2,  0.0),
   'sirius':            (0.0,  0.7,  0.8),
   'soromid':           (0.95, 0.5,  0.0),
   'frontier':          (0.8,  0.8,  0.0),
   'pirate':            (1.0,  0.0,  0.0),
   'independent':       (0.0,  0.0,  1.0),
   'proteron':          (0.9,  0.0,  0.9),
   'thurion':           (0.5,  0.5,  0.5),
   'collective':        (0.8,  0.8,  0.8),
   'goddard':           (0.0,  0.1,  1.0),
   'traders_society':   (0.0,  0.1,  1.0),
   'orez':              (0.0,  0.4,  0.9),
   'flf':               (0.9,  0.9,  0.0),
}

for f in ['wild_ones', 'raven_clan', 'dreamer_clan', 'black_lotus', 'lost', 'marauder']:
   faction_color[f] = faction_color['pirate']

def get_spob_faction( nam ):
   T = ET.parse(getpath(PATH, "spob", nam+".xml")).getroot()
   e = T.find("./presence/faction")
   return nam2base(e.text) if e is not None else None


# V, E, pos, tradelane, color = xml_files_to_graph(args, use_color)
def xml_files_to_graph( args, get_colors = False ):
   name2id = dict()
   name, acc, pos, tradelane, color = [], [], [], set(), dict()

   for i in range(len(args)):
      bname = basename(args[i])
      if bname[-4:] != '.xml':
         stderr.write('err: "' + args[i] + '"\n')
         continue

      bname = bname[:-4]
      name.append(bname)
      T=ET.parse(args[i]).getroot()

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
         for e in T.findall('spobs/spob'):
            if f := get_spob_faction(nam2base(e.text)):
               if f not in fact:
                  fact[f] = 0
               fact[f] += 1

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
            stderr.write('no target defined in "'+args[i]+'"jump#'+str(count)+'\n')
      count += 1

   n2i = lambda x:name2id[x]
   ids = [n2i(x) for x in name]
   acc = [[(n2i(t[0]),t[1]) for t in L] for L in acc]
   return dict(zip(ids,name)), dict(pos), dict(zip(ids,acc)), tradelane, color

if __name__ == '__main__':
   from sys import argv, exit, stdout, stderr, stdin
   import os

   def do_reading():
      for arg in os.listdir(PATH):
         if arg[-4:] != '.xml':
            continue

         bname = arg[:-4]
         T = ET.parse(os.path.join(PATH, arg)).getroot()
         try:
            name = T.attrib['name']
         except:
            stderr.write('no name defined in "' + bname + '"\n')
         try:
            e = T.find('pos')
            x, y = (e.attrib['x'], e.attrib['y'])
         except:
            stderr.write('no position defined in "' + bname + '"\n')
         print(bname, x, y, name)

   def do_writing( scale = 1.0 ):
      from ssys import fil_ET
      for l in stdin:
         if (line := l.strip()) != '':
            (n, x, y) = l.strip().split(" ")[:3]
            name = os.path.join(PATH, n+'.xml')
            T = fil_ET(name)
            e = T.getroot().find('pos')
            x = str(float(x) * scale)
            y = str(float(y) * scale)
            e.attrib['x'], e.attrib['y'] = x, y
            T.write(name)

   if do_write := ('-w' in argv[1:]):
      argv.remove('-w')
      if argv[1:] != []:
         scale = float(argv.pop(1))
      else:
         scale = 1.0

   if argv[1:] != []:
      ok = '-h' in argv or '--help' in argv[1:]
      fp = stdout if ok else stderr
      fp.write('usage:  ' + os.path.basename(argv[0]) + ' [-w [scale]]\n')
      fp.write('  Lists (ssys, x, y, name) for all ssys in dat/ssys.\n')
      fp.write('  If -w is set, reads (ssys, x, y, ...) on stdin and update dat/ssys.\n')
      exit(0 if ok else 1)

   getpath = lambda *x: os.path.realpath(os.path.join(*x))
   script_dir = os.path.dirname(__file__)
   PATH = getpath(script_dir, '..', '..', 'dat', 'ssys')

   if do_write:
      do_writing(scale)
   else:
      do_reading()
