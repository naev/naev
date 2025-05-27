#!/usr/bin/env python3

import xml.etree.ElementTree as ET
from sys import argv

args = argv[1:]

from math import cos, sin, pi, sqrt

class vec(tuple):
   def __add__(self, other):
      return vec([x+y for (x,y) in zip (self,other)])
   def __sub__(self, other):
      return self+(other*-1.0)
   def __mul__(self, other):
      return vec([x*other for x in self])
   def __truediv__(self, other):
      return self*(1.0/other)
   def __round__(self, dig = 0):
      return vec([round(x, dig) for x in self])
   def __str__(self):
      return str(tuple([int(a) if int(a) == a else a for a in self]))
   def rotate(self, a):
      return vec((self[0]*cos(a)-self[1]*sin(a), self[0]*sin(a)+self[1]*cos(a)))
   def size(self):
      return sqrt(sum([c*c for c in self]))
   def normalize(self):
      return self/self.size()
   def to_dict(self):
      return {'x':self[0], 'y':self[1]}

class starmap(dict):
   def __getitem__(self, key):
      if key not in self:
         PATH = 'dat/ssys'
         name = PATH + '/' + key + '.xml'
         T=ET.parse(name).getroot()

         for e in T.findall("pos"):
            try:
               self[key] = vec((float(e.attrib['x']),float(e.attrib['y'])))
            except:
               stderr.write('no position defined in "'+basename+'"\n')
               self[key] = None
            break

      return dict.__getitem__(self, key)

sm = starmap()

def nam2fil(nam):
   nam = nam.strip().lower()
   for t in [(' ','_'), ("'",'')]:
      nam = nam.replace(*t)
   return nam

for i in args:
   changed = False
   p = ET.parse(i)
   T = p.getroot()
   myname = nam2fil(T.attrib["name"])

   for e in T.findall("general/radius"):
      radius = float(e.text)
      break

   for e in T.findall("jumps/jump"):
      dst = nam2fil(e.attrib['target'])
      for f in e.findall("autopos"):
         changed = True
         f.tag = "pos"
         v = (sm[dst] - sm[myname]).normalize()*radius
         for k, v in v.to_dict().items():
            f.set(k, str(v))
         f.set('was_auto','true')

   if changed:
      p.write(i)
      

