#!/usr/bin/env python3


import xml.etree.ElementTree as ET
from sys import argv, stderr
from math import cos, sin, pi, sqrt

import os
script_dir = os.path.dirname( __file__ )
PATH = os.path.realpath(os.path.join( script_dir, '..', '..', 'dat', 'ssys'))

class vec(tuple):
   def __add__( self, other ):
      return vec([x+y for (x,y) in zip(self,other)])

   def __sub__( self, other ):
      return self+(other*-1.0)

   def __mul__( self, other ):
      return vec([x*other for x in self])

   def __truediv__( self, other ):
      return self*(1.0/other)

   def __round__( self, dig = 0 ):
      return vec([round(x, dig) for x in self])

   def __str__( self ):
      return str(tuple([int(a) if int(a) == a else a for a in self]))

   def rotate( self, a ):
      return vec((self[0]*cos(a)-self[1]*sin(a), self[0]*sin(a)+self[1]*cos(a)))

   def size( self ):
      return sqrt(sum([c*c for c in self]))

   def normalize( self ):
      return self/self.size()

   def to_dict( self ):
      return {'x':self[0], 'y':self[1]}

def sys_fil( nam ):
   return os.path.join(PATH, nam + '.xml')

import subprocess
cmd = os.path.realpath(os.path.join( script_dir, '..', 'repair_xml.sh'))
def sys_fil_ET( name ):
   T = ET.parse(name)
   oldw = T.write
   def write( nam ):
      oldw(nam)
      subprocess.run( [cmd, nam])
   T.write = write
   return T

class starmap(dict):
   def __getitem__( self, key ):
      if key not in self:
         name = sys_fil(key)
         T = ET.parse(name).getroot()
         for e in T.findall("pos"):
            try:
               self[key] = vec((float(e.attrib['x']),float(e.attrib['y'])))
            except:
               stderr.write('no position defined in "'+basename+'"\n')
               self[key] = None
            break
      return dict.__getitem__(self, key)

def sysnam2sys( nam ):
   nam = nam.strip().lower()
   for t in [(' ','_'), ("'",'')]:
      nam = nam.replace(*t)
   return nam


