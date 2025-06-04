
from sys import stderr
import xml.etree.ElementTree as ET
from os.path import basename
from ssys import nam2base, getpath, PATH


def get_spob_faction(nam):
   T = ET.parse(getpath(PATH, "spob", nam+".xml")).getroot()
   e = T.find("./presence/faction")
   return e.text if e is not None else None


# V, E, pos, tradelane, factions = xml_files_to_graph(args)
def xml_files_to_graph(args):
   name2id = dict()
   name, acc, pos, tradelane, faction = [], [], [], set(), dict()

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

      fact = dict()
      for e in T.findall('spobs/spob'):
         if f := get_spob_faction(nam2base(e.text)):
            f = nam2base(f)
            if f not in fact:
               fact[f] = 0
            fact[f] += 1

      fact = list(sorted([(n,f) for (f,n) in fact.items()], reverse = True))
      #print(bname + ':' + str(fact))
      if len(fact)>1 and fact[0][1]=='independant' and fact[0][0] == fact[1][0]:
         faction[bname] = fact[1][1]
      else:
         faction[bname] = (fact+[(None,None)])[0][1]
 
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
   return dict(zip(ids,name)), dict(zip(ids,acc)), dict(pos), tradelane, faction

