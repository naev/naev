#!/usr/bin/env python3


import xml.etree.ElementTree as ET
from sys import argv, stderr
from math import cos, sin, pi, sqrt

import os
script_dir = os.path.dirname(__file__)
PATH = os.path.realpath(os.path.join(script_dir, '..', '..', 'dat', 'ssys'))

class _vec(tuple):
   def __add__( self, other ):
      return _vec([x+y for (x, y) in zip(self, other)])

   def __sub__( self, other ):
      return self + other*-1.0

   def _rotate( self, sa, ca ):
      return _vec((self[0]*ca-self[1]*sa, self[0]*sa+self[1]*ca))

   def __mul__( self, other ):
      if isinstance(other, _vec):
         """
         Dot product
         """
         return sum([a*b for (a,b) in zip(self, other)])
      elif isinstance(other, transf):
         """
         Apply transformation
         """
         sa = other.vec
         return self._rotate(sa, sqrt(1.0 - sa*sa)) * other.fact
      else:
         """
         External product
         """
         return _vec([x*other for x in self])

   def __neg__( self ):
      return self * -1.0

   def __truediv__( self, other ):
      if isinstance(other, _vec):
         """
         The transformation that turns other into self.
         """
         return transf(other, self)
      else:
         return self * (1.0/other)

   def __round__( self, dig = 0 ):
      return _vec([round(x, dig) for x in self])

   def __str__( self ):
      return str(tuple([int(a) if int(a) == a else a for a in self]))

   def rotate( self, degrees ):
      angle = degrees / 180.0 * pi
      return self._rotate(sin(angle), cos(angle))

   def size_sq( self ):
      return self*self

   def size( self ):
      return sqrt(self.size_sq())

   def normalize( self, new_size = 1.0 ):
      return self / self.size() * new_size

   def to_dict( self ):
      return {'x':self[0], 'y':self[1]}

class transf:
   """
   A rotation and a scaling.
   Defined by a pair of vectors (before trans, after trans)
   """
   def __init__( self, v1, v2 ):
      l1 = v1.size()
      if v1*v2 < 0:
         l1 = -l1
         v1 = -v1
      self.fact = v2.size()/l1
      v1 = v1.normalize()
      v2 = v2.normalize()
      self.vec = v1[0]*v2[1] - v1[1]*v2[0]
   def __mul__( self, other ):
      """
      Application
      """
      if not isinstance(other, _vec):
         raise Exception('transf only applies to vec')
      return other*self
   def __add__( self, other ):
      """
      Composition
      """
      if not isinstance(other, transf):
         raise Exception('transf only adds with itself')
      v1 = _vec(1.0, 0.0)
      v2 = v1 * self * other
      return transf(v1, v2)

def vec( *args ):
   if len(args) == 0:
      return _vec((0.0, 0.0))
   else:
      if len(args) == 1:
         args = args[0]
      return _vec((float(x) for x in args))

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
         for e in T.findall('pos'):
            try:
               self[key] = _vec(float(e.attrib['x']), float(e.attrib['y']))
            except:
               stderr.write('no position defined in "' + name + '"\n')
               self[key] = None
            break
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
         acc.append((sysnam2sys(e.attrib['target']), False))
         for f in e.findall('hidden'):
            acc[-1]=(acc[-1][0], True)
            break
      except:
         stderr.write('no target defined in "'+sys+'"jump#'+str(count)+'\n')
      count += 1
   return acc

