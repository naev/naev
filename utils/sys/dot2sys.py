#!/usr/bin/env python3

from sys import stdin, stderr
from math import pi

from ssys import vec, sys_fil_ET, sys_fil, sysnam2sys, sysneigh


# positions
d = dict()

def process( lin ):
   if len(lin.split("--")) != 1:
      return
   lin = lin.strip()
   for c in ['\n','\t',3*' ',2*' ']:
      lin = lin.replace(c,' ')

   if (nam := lin.split(' ')[0]) in ['graph', 'edge', 'node']:
      return
   pos = lin.split('pos="')[1].split('"')[0].split(",")
   x, y = float(pos[0]), float(pos[1])
   d[nam] = vec((x,y))

acc = ''
for line in stdin:
   n = line.find(']')
   acc += line[:n]
   if n != -1:
      process(acc)
      acc = ''

# bounding boxes
class bb:
   def __init__( self ):
      self.empty = True

   def __iadd__( self, t ):
      if self.empty:
         (self.maxx, self.maxy) = t
         (self.minx, self.miny) = t
         self.empty = False
      else:
         if self.minx > t[0]:
            self.minx = t[0]
         elif self.maxx < t[0]:
            self.maxx = t[0]
         if self.miny > t[1]:
            self.miny = t[1]
         elif self.maxy < t[1]:
            self.maxy = t[1]
      return self

   def mini( self ):
      return vec((self.minx,self.miny)) if not self.empty else None

   def maxi( self ):
      return vec((self.maxx,self.maxy)) if not self.empty else None

   def __str__( self ):
      return "["+str(round(self.mini(), 6))+" "+str(round(self.maxi(), 6))+"]"


bbox = bb()

for k in d:
   d[k] *= 3.0/2.0
   bbox += d[k]

oldbb = bb()
for k in d:
   if k[0]!="_":
      nam = k
      if nam[0] == '"':
         nam = nam[1:-1]
      o = sys_fil_ET(sys_fil(nam))
      T = o.getroot()
      for e in T.findall("pos"):
         oldbb += (float(e.attrib['x']), float(e.attrib['y']))
         break

again = bb()
for k in d:
   d[k]-= bbox.mini()
   d[k]+= oldbb.mini()
   again+= d[k]

stderr.write(str(oldbb)+" -> "+str(bbox)+"\n")
stderr.write(" -> "+str(again)+"\n")


# Post - processing

small = [
   ('carnis_minor', 'carnis_major', 0.7),
   ('gliese', 'gliese_minor', 0.5),
   ('kruger', 'krugers_pocket', 0.5)
]
for (i,j,q) in small:
   a = d[i]*q + d[j]*(1.0-q)
   d[i] = (d[i]+a) / 2.0
   d[j] = (d[j]+a) / 2.0


d['syndania'] = vec((d['syndania'][0],d['stint'][1]))

Spir = ['syndania', 'nirtos', 'sagittarius', 'hopa', 'scholzs_star', 'veses', 'alpha_centauri', 'padonia']

Scenter = (d[Spir[0]]+d[Spir[4]])*(1.0/2.0)
v = (d[Spir[0]] - Scenter) * 0.75
for i,s in enumerate(Spir):
   rad = pow(1.25,-(i%4))
   d[s] = Scenter + v.rotate(-i*pi/4)*rad

d['urillian'] = Scenter + (v.rotate(-4.5*pi/4)*pow(1.25,-4.5))
d['baitas'] = Scenter + (v.rotate(-8.5*pi/4)*pow(1.25,-4.5))
d['protera'] = Scenter + (v.rotate(-2.5*pi/4)*pow(1.25,-8))
d['tasopa'] = Scenter + (v.rotate(-6.5*pi/4)*pow(1.25,-8))

d['possum'] += (d['starlight_end']-d['possum'])*0.5
d['starlight_end'] += (d['starlight_end']-d['possum'])*1.5

v = d['hystera']-d['leporis']
u = v.rotate(-pi/3)
d['korifa'] = d['hystera'] + u
d['apik'] = d['leporis'] + u
d['telika'] = d['apik'] - v
d['mida'] = d['apik'] + u
d['ekta'] = d['mida'] - v
d['akra'] = d['mida'] + u

"""
from median import median
def rebalance(sys):
   d[sys] = median([s for (s,_) in sysneigh(sys)])
"""

v = (d['possum']-d['moor'])/3.0
for i in ['stint', 'moor', 'taxumi', 'longbow', 'herculis', 'starlight_end']:
   d[i] += v

d['taxumi'] += (d['starlight_end']-d['taxumi']) / 3.0
d['norn'] += (d['pisces_prime']-d['bonanza']) / 3.0
d['ngc1317'] += (d['ngc1317']-d['stelman']) / 3.0
d['reptile'] += (d['newmarch']-d['armorhead']) / 6.0
v = (d['aesir']-d['vanir']) / 4.0
d['aesir'] += v
d['vanir'] += v


# Smoothen tradelane
tradelane = set()
for k in d:
   if k[0] != '_':
      T = sys_fil_ET(sys_fil(k)).getroot()
      for e in T.findall("tags/tag"):
         if e.text == 'tradelane':
            tradelane.add(k)
            break

newp = dict()
for k in d:
   if k[0] != '_' and (k in tradelane):
      tln = [s for (s,_) in sysneigh(k) if (s in tradelane)]
      if (n := len(tln)) > 1:
         p = sum([d[s] for s in tln], vec((0,0)))
         newp[k] = d[k]*(1.0-n*0.125) + p*0.125

for k, v in newp.items():
   d[k] = v


# Apply to ssys/

for k in d:
   d[k] = round(d[k], 9)
   if k[0]!="_":
      nam = k
      if nam[0] == '"':
         nam = nam[1:-1]
      nam = sys_fil(nam)
      o = sys_fil_ET(nam)
      T = o.getroot()
      for e in T.findall("pos"):
         e.set('x', str(d[k][0]))
         e.set('y', str(d[k][1]))
         break
      o.write(nam)


