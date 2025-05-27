#!/usr/bin/env python3

from sys import stdin, stderr

acc = []

fact = 72*3/2
d = dict()

class vec(tuple):
   def __add__(self, other):
      return vec([x+y for (x,y) in zip (self,other)])
   def __sub__(self, other):
      return self+(other*-1.0)
   def __mul__(self, other):
      return vec([x*other for x in self])
   def __div__(self, other):
      return self*(1.0/other)
   def __round__(self, dig = 0):
      return vec([round(x, dig) for x in self])
   def __str__(self):
      return str(tuple([int(a) if int(a) == a else a for a in self]))

def process( lin ):
   if len(lin.split("--")) != 1:
      return
   lin = lin.strip()
   for c in ['\n','\t',3*' ',2*' ']:
      lin = lin.replace(c,' ')

   if (nam := lin.split(' ')[0]) in ['graph','edge','node']:
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

class bb:
   def __init__(self):
      self.empty=True

   def __iadd__(self, t):
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

   def mini(self):
      return vec((self.minx,self.miny)) if not self.empty else None

   def maxi(self):
      return vec((self.maxx,self.maxy)) if not self.empty else None

   def __str__(self):
      return "["+str(round(self.mini()))+" "+str(round(self.maxi()))+"]"


bbox = bb()

for k in d:
   d[k] *= 4.5/3.0
   bbox += d[k]


# Post - processing

small = [('carnis_minor','carnis_major'), ('gliese','gliese_minor'), ('kruger','krugers_pocket')]
for (i,j) in small:
   a=(d[i]+d[j])*(1.0/2.0)
   d[i]=(d[i]*3+a*2)*(1.0/5.0)
   d[j]=(d[j]*3+a*2)*(1.0/5.0)


from math import cos, sin, pi

def rotate(v,a):
   return vec((v[0]*cos(a)-v[1]*sin(a), v[0]*sin(a)+v[1]*cos(a)))

Spir = ['syndania', 'nirtos', 'sagittarius', 'hopa', 'scholzs_star', 'veses', 'alpha_centauri', 'padonia']

Scenter = (d[Spir[0]]+d[Spir[4]])*(1.0/2.0)
v = (d[Spir[0]] - Scenter) * 0.7
for i,s in enumerate(Spir):
   rad = pow(1.15,-(i%4))
   d[s] = Scenter + rotate(v,-i*pi/4)*rad

d['urillian'] = Scenter + (rotate(v,-4*pi/4)*0.38)
d['baitas'] = Scenter + (rotate(v,-8*pi/4)*0.38)
d['protera'] = Scenter + (rotate(v,-2*pi/4)*0.22)
d['tasopa'] = Scenter + (rotate(v,-6*pi/4)*0.22)

d['possum']+=(d['starlight_end']-d['possum'])*0.5
d['starlight_end']+=(d['starlight_end']-d['possum'])*1.5

v = d['hystera']-d['leporis']
u = rotate(v,-pi/3)
d['korifa'] = d['hystera'] + u
d['apik'] = d['leporis'] + u
d['telika'] = d['apik'] - v
d['mida'] = d['apik'] + u
d['ekta'] = d['mida'] - v
d['akra'] = d['ekta'] + v


import xml.etree.ElementTree as ET


def reb(sys):
   acc = []
   fp="dat/ssys/"+sys+".xml"
   #print(fp)
   o = ET.parse(fp)
   T = o.getroot()
   for e in T.findall("./jumps/jump"):
      acc.append(e.attrib['target'])
   tot = vec((0,0))
   for i in acc:
      nam = i.lower().replace(' ','_').replace("'",'')
      tot += d[nam]
   tot *= 1.0/len(acc)
   d[sys] = tot

#reb('cebus')

#v = d['aesria'] - d['vean']
#d['flow'] = d['vean'] + rotate(v,-pi/3)


# Apply to ssys/

oldbb = bb()
for k in d:
   if k[0]!="_":
      nam = k
      if nam[0] == '"':
         nam = nam[1:-1]
      fp = "dat/ssys/" + nam + ".xml"
      o = ET.parse(fp)
      T = o.getroot()
      for e in T.findall("pos"):
         oldbb += (float(e.attrib['x']),float(e.attrib['y']))
         break

stderr.write(str(oldbb)+"->"+str(bbox)+"\n")
diff = bbox.mini() - oldbb.mini()
again = bb()

for k in d:
   d[k]-= bbox.mini()
   d[k]+= oldbb.mini()
   again+= d[k]

stderr.write("->"+str(again)+"\n")

for k in d:
   d[k] = round(d[k],3)
   if k[0]!="_":
      nam = k
      if nam[0] == '"':
         nam = nam[1:-1]
      fp = "dat/ssys/" + nam + ".xml"
      o = ET.parse(fp)
      T = o.getroot()
      for e in T.findall("pos"):
         e.set('x', str(d[k][0]))
         e.set('y', str(d[k][1]))
         break
      o.write(fp)


