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

# Vnames, Vpos, E, tradelane = xml_files_to_graph(args)
def xml_files_to_graph( args = None ):
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
   return dict(zip(ids,name)), dict(pos), dict(zip(ids,acc)), tradelane

