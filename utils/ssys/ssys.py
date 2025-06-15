# python3


import os
import sys
import xml.etree.ElementTree as ET
from sys import argv, stderr

getpath = lambda *x: os.path.realpath(os.path.join(*x))

script_dir = os.path.dirname(__file__)
sys.path.append(getpath(script_dir, '..'))

from xml_name import xml_name as nam2base

PATH = getpath(script_dir, '..', '..', 'dat')

from geometry import vec


_fil =lambda folder: lambda nam : getpath(PATH, folder, nam + '.xml')
spob_fil = _fil('spob')
ssys_fil = _fil('ssys')


import subprocess
cmd = getpath(script_dir, '..', 'repair_xml.sh')
need_repair = []
from atexit import register

def fil_ET( name ):
   T = ET.parse(name)
   oldw = T.write
   def write( nam ):
      global need_repair
      oldw(nam)
      if need_repair == []:
         def _repair_ET():
            global need_repair
            if need_repair != []:
               subprocess.run([cmd] + need_repair)
            need_repair = []
         register(_repair_ET)
      need_repair.append(nam)
   T.write = write
   return T

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

def ssysneigh( sys ):
   T = ET.parse(ssys_fil(sys)).getroot()
   acc = []
   count = 1
   for e in T.findall('./jumps/jump'):
      try:
         acc.append((nam2base(e.attrib['target']), e.find('hidden') is not None))
      except:
         stderr.write('no target defined in "'+sys+'" jump '+str(count)+'\n')
      count += 1
   return acc

def vec_from_element( e ):
   return vec(float(e.attrib['x']), float(e.attrib['y']))

def vec_to_element( e, v ):
   e.set('x', str(v[0]))
   e.set('y', str(v[1]))
