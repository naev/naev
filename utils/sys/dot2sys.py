#!/usr/bin/env python3

if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')


from sys import stdin, stderr

from ssys import vec, sys_fil_ET, sys_fil, sysnam2sys, sysneigh


# positions
pos = dict()

def process( lin ):
   if len(lin.split("--")) != 1:
      return
   lin = lin.strip()
   for c in ['\n', '\t', 3*' ', 2*' ']:
      lin = lin.replace(c, ' ')

   if (nam := lin.split(' ')[0]) in ['graph', 'edge', 'node']:
      return
   position = lin.split('pos="')[1].split('"')[0].split(",")
   x, y = float(position[0]), float(position[1])
   pos[nam] = vec(x, y)

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
      return vec(self.minx,self.miny) if not self.empty else None

   def maxi( self ):
      return vec(self.maxx,self.maxy) if not self.empty else None

   def __str__( self ):
      return str(round(self.mini()))+":"+str(round(self.maxi()))


bbox = bb()
oldbb = bb()

for k in pos:
   pos[k] *= 3.0/2.0
   bbox += pos[k]
   if k[0] != "_":
      nam = k
      if nam[0] == '"':
         nam = nam[1:-1]
      T = sys_fil_ET(sys_fil(nam)).getroot()
      for e in T.findall("pos"):
         oldbb += (float(e.attrib['x']), float(e.attrib['y']))
         break

again = bb()
for k in pos:
   pos[k] += oldbb.mini() - bbox.mini()
   again += pos[k]

stderr.write(str(oldbb) + " -> " + str(bbox) + "\n")
stderr.write(" -> " + str(again) + "\n")


# Post - processing

small = [
   ('carnis_minor', 'carnis_major', 0.7),
   ('gliese', 'gliese_minor', 0.5),
   ('kruger', 'krugers_pocket', 0.5)
]
for (i,j,q) in small:
   a = pos[i]*q + pos[j]*(1.0-q)
   pos[i] = (pos[i]+a) / 2.0
   pos[j] = (pos[j]+a) / 2.0


pos['syndania'] = vec(pos['syndania'][0], pos['stint'][1])

Spir = ['syndania', 'nirtos', 'sagittarius', 'hopa', 'scholzs_star', 'veses', 'alpha_centauri', 'padonia']

Scenter = (pos[Spir[0]]+pos[Spir[4]]) / 2.0
v = (pos[Spir[0]] - Scenter) * 0.75
for i,s in enumerate(Spir):
   rad = pow(1.25,-(i%4))
   pos[s] = Scenter + v.rotate(-i*45)*rad

pos['urillian']   = Scenter + (v.rotate(-4.5*45)*pow(1.25,-4.5))
pos['baitas']     = Scenter + (v.rotate(-8.5*45)*pow(1.25,-4.5))
pos['protera']    = Scenter + (v.rotate(-2.5*45)*pow(1.25,-8.0))
pos['tasopa']     = Scenter + (v.rotate(-6.5*45)*pow(1.25,-8.0))

def toward(src, dst, q):
   global pos
   pos[src] += (pos[dst]-pos[src]) * q

toward('possum', 'starlight_end', 0.5)
toward('starlight_end', 'possum', -1.5)

v = pos['hystera'] - pos['leporis']
pos['leporis'] = pos['haered'] + (pos['leporis']-pos['haered']).normalize(v.size())
pos['hystera'] = pos['leporis'] + v

u = v.rotate(-60)
pos['korifa'] = pos['hystera'] + u
pos['apik'] = pos['leporis'] + u
pos['telika'] = pos['apik'] - v
pos['mida'] = pos['apik'] + u
pos['ekta'] = pos['mida'] - v
pos['akra'] = pos['mida'] + u


"""
from median import median
def rebalance(sys):
   pos[sys] = median([s for (s,_) in sysneigh(sys)])
"""

v = (pos['possum']-pos['moor']) / 3.0
for i in ['stint', 'moor', 'taxumi', 'longbow', 'herculis', 'starlight_end']:
   pos[i] += v

toward('taxumi', 'starlight_end', 1.0/3.0)
toward('ngc1317', 'stelman', -1.0/3.0)

pos['norn'] += (pos['pisces_prime']-pos['bonanza']) / 3.0
pos['reptile'] += (pos['newmarch']-pos['armorhead']) / 6.0

v = (pos['aesir']-pos['vanir']) / 4.0
pos['aesir'] += v
pos['vanir'] += v

pos['dohriabi'] += (pos['dohriabi']-pos['overture']) / 4.0
pos['anubis_black_hole'] += (pos['ngc13674']-pos['ngc1562']) / 8.0

l = (pos['ngc2601'] - pos['anubis_black_hole']).size()
v = (pos['ngc2601'] + pos['ngc11935'] - pos['anubis_black_hole']*2.0).normalize()
pos['zied'] = pos['anubis_black_hole'] + v*l

v1 = pos['ngc7078'] - pos['anubis_black_hole']
v2 = pos['octavian'] - pos['anubis_black_hole']
v = pos['ngc7533'] - pos['anubis_black_hole']

v = v.normalize(((v1.size() + v2.size())/2.0))
pos['ngc7533'] = pos['anubis_black_hole'] + v


# Smoothen tradelane

tradelane = set()
for k in pos:
   if k[0] != '_':
      T = sys_fil_ET(sys_fil(k)).getroot()
      for e in T.findall("tags/tag"):
         if e.text == 'tradelane':
            tradelane.add(k)
            break

newp = dict()
for k in pos:
   if k[0] != '_' and (k in tradelane):
      tln = [s for (s, _) in sysneigh(k) if (s in tradelane)]
      if (n := len(tln)) > 1:
         p = sum([pos[s] for s in tln], vec())
         newp[k] = pos[k]*(1.0-n*0.125) + p*0.125

for k, v in newp.items():
   pos[k] = v


# Increase terminal ngc dist to neighbour to at least average edge length.

total = 0.0
count = 0
for k in pos:
   if k[0] != '_':
      for n in [s for (s, _) in sysneigh(k) if (s in tradelane)]:
         total += (pos[n]-pos[k]).size()
         count += 1
avg = total / count

for k in pos:
   if k[:3] == 'ngc' and k[3:] not in ['22375', '20489', '4746', '9415']:
      if len(n := sysneigh(k)) == 1:
         n = n[0][0]
         v = pos[k] - pos[n]
         if v.size() < avg:
            pos[k] = pos[n] + v.normalize(avg)


# Apply to ssys/

off = (pos['dohriabi']-pos['anubis_black_hole']) / 2.0
for k in pos:
   pos[k] += off
   pos[k] = round(pos[k], 9)
   if k[0] != "_":
      nam = k
      if nam[0] == '"':
         nam = nam[1:-1]
      nam = sys_fil(nam)
      o = sys_fil_ET(nam)
      for e in o.getroot().findall("pos"):
         e.set('x', str(pos[k][0]))
         e.set('y', str(pos[k][1]))
         break
      o.write(nam)


