#!/usr/bin/env python3


import os
import xml.etree.ElementTree as ET
from sys import argv, stderr, path

getpath = lambda *x: os.path.realpath(os.path.join(*x))

script_dir = os.path.dirname(__file__)
path.append(getpath(script_dir, '..'))

from xml_name import xml_name as nam2base
from naev_xml import naev_xml, xml_node

PATH = getpath(script_dir, '..', '..', 'dat')

from geometry import vec


_fil =lambda folder: lambda nam : getpath(PATH, folder, nam + '.xml')
spob_fil = _fil('spob')
ssys_fil = _fil('ssys')

AVAILABLE_FIELDS = ['jump', 'spob', 'asteroid', 'waypoint']
MANDATORY_FIELDS = ['jump', 'spob']
OTHER_FIELDS = set(AVAILABLE_FIELDS) - set(MANDATORY_FIELDS)
class ssys_xml(naev_xml):
   def __init__(self, fnam= None, **kwargs):
      naev_xml.__init__(self, fnam, **kwargs)
      if fnam is None:
         self['ssys'] = {}
      elif 'ssys' not in self:
         raise Exception('Invalid ssys filename "' + fnam + '"')
      s = self['ssys']
      for f in AVAILABLE_FIELDS:
         fs = f + 's'
         if not s.get(fs):
            s[fs] = {f: []}
         elif f not in s[fs]:
            s[fs][f] = []
         elif not isinstance(s[fs][f], list):
            # don't deepcopy
            dict.__setitem__(s[fs], f, [s[fs][f]])
      self._uptodate = True

   def save(self, *args, **kwargs):
      ssys = self['ssys']
      utd = self._uptodate
      for s in OTHER_FIELDS:
         if ssys[s + 's'] == {s: []}:
            del ssys[s + 's']
      self._uptodate = utd
      res = naev_xml.save(self, *args, **kwargs)
      for s in OTHER_FIELDS:
         if s + 's' not in ssys:
            ssys[s + 's'] = {s: []}
      self._uptodate = True
      return res

class starmap(dict):
   def __missing__( self, key ):
      name = ssys_fil(key)
      T = ET.parse(name).getroot()
      if (e := T.find('pos')) is not None:
         try:
            self[key] = vec(float(e.attrib['x']), float(e.attrib['y']))
         except:
            stderr.write('no position defined in "' + name + '"\n')
            self[key] = None
      return self[key]

def pos_to_vec( e ):
   return vec(e['$@x'], e['$@y'])

def vec_to_pos( v ):
   return {'$@x': v[0], '$@y': v[1] }

if __name__ == '__main__':
   from sys import argv

   if inplace := '-i' in argv[1:]:
      argv.remove('-i')

   if '-h' in argv[1:]:
      stderr.write(
         'usage: ' + argv[0].split('/')[-1] + '  -i  [ file.xml.. ]\n'
         '  Reads a ssys xml file, formats it and outputs the result.\n'
         '  If -i is set, does it in place.\n'
      )
      exit(0)
   for i in argv[1:]:
      s = ssys_xml(i)
      if inplace:
         s.touch()
      else:
         s.save_as('-')
      s.save()
