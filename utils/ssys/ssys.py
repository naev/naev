# python3


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

class ssys_xml(naev_xml):
   def __init__(self, *args):
      naev_xml.__init__(self, *args)
      if 'ssys' not in self:
         raise Exception('Invalid ssys filename "' + repr(filename) + '"')
      s = self['ssys']
      for f in ['jump', 'spob', 'asteroid', 'waypoint']:
         fs = f + 's'
         if not s.get(fs):
            s[fs] = {f: []}
         elif f not in s[fs]:
            s[fs][f] = []
         elif not isinstance(s[fs][f], list):
            # don't deepcopy
            dict.__setitem__(s[fs], f, [s[fs][f]])
      self._uptodate = True

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
