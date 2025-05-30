#!/usr/bin/env python3


import xml.etree.ElementTree as ET
from sys import argv, stderr
import os
script_dir = os.path.dirname(__file__)
PATH = os.path.realpath(os.path.join(script_dir, '..', '..', 'dat', 'ssys'))
PATH_SPOB = os.path.realpath(os.path.join(script_dir, '..', '..', 'dat', 'spob'))

from geometry import vec

def spob_fil( nam ):
   if nam[0] == '"' and nam[-1]== '"':
      nam = nam[1:-1]
   return os.path.join(PATH_SPOB, nam + '.xml')

def sys_fil( nam ):
   if nam[0] == '"' and nam[-1]== '"':
      nam = nam[1:-1]
   return os.path.join(PATH, nam + '.xml')

import subprocess
cmd = os.path.realpath(os.path.join(script_dir, '..', 'repair_xml.sh'))
need_repair = []
from atexit import register

def sys_fil_ET( name ):
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
   def __getitem__( self, key ):
      if key not in self:
         name = sys_fil(key)
         T = ET.parse(name).getroot()
         if (e := T.find('pos')) is not None:
            try:
               self[key] = vec(float(e.attrib['x']), float(e.attrib['y']))
            except:
               stderr.write('no position defined in "' + name + '"\n')
               self[key] = None
      return dict.__getitem__(self, key)

def sysnam2sys( nam ):
   nam = nam.strip()
   for t in [(' ', '_'), ("'s", 's'), ("'", "\'"),  ('C-', 'C')]:
      nam = nam.replace(*t)
   return nam.lower()

def sysneigh(sys):
   T = ET.parse(sys_fil(sys)).getroot()
   acc = []
   count = 1
   for e in T.findall('./jumps/jump'):
      try:
         acc.append((sysnam2sys(e.attrib['target']), e.find('hidden') is not None))
      except:
         stderr.write('no target defined in "'+sys+'"jump#'+str(count)+'\n')
      count += 1
   return acc

