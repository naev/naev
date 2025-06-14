#!/usr/bin/env python3


from sys import stdin, stderr, argv, exit
from ssys import vec, fil_ET, ssys_fil, ssysneigh, vec_from_element, vec_to_element
from geometry import bb


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

if argv[1:] != []:
   stderr.write('usage: '+argv[0]+'\n')
   stderr.write('  Reads a dot file on stdin, applies a pre-processing,\n')
   stderr.write('  and then applies it to ssys.\n')
   exit(0)


# positions
pos = dict()

def process( lin ):
   if len(lin.split('--')) != 1:
      return
   lin = lin.strip()
   for c in ['\n', '\t', 3*' ', 2*' ']:
      lin = lin.replace(c, ' ')

   if (nam := lin.split(' ')[0]) in ['graph', 'edge', 'node']:
      return
   position = lin.split('pos="')[1].split('"')[0].split(',')
   x, y = float(position[0]), float(position[1])
   pos[nam] = vec(x, y)

acc = ''
for line in stdin:
   n = line.find(']')
   acc += line[:n]
   if n != -1:
      process(acc)
      acc = ''

bbox = bb()
oldbb = bb()

for k in pos:
   pos[k] *= 3.0/2.0
   bbox += pos[k]
   if k[0] != '_':
      nam = k
      if nam[0] == '"':
         nam = nam[1:-1]
      T = fil_ET(ssys_fil(nam)).getroot()
      if (e := T.find('pos')) is not None:
         oldbb += vec_from_element(e)

again = bb()
for k in pos:
   pos[k] += oldbb.mini() - bbox.mini()
   again += pos[k]

stderr.write(str(oldbb) + ' -> ' + str(bbox))
stderr.write(' -> ' + str(again) + '\n')


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

v = (pos['urillian'] - pos['sagittarius']) + (pos['haered']-pos['cleai'])/6.0
for i in Spir + ['urillian', 'baitas', 'protera', 'tasopa']:
   pos[i] += v

def toward(src, dst, q):
   global pos
   pos[src] += (pos[dst]-pos[src]) * q

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


#v = (pos['possum']-pos['moor']) / 3.0
#for i in ['stint', 'moor', 'taxumi', 'longbow', 'herculis', 'starlight_end']:
#   pos[i] += v
toward('taxumi', 'starlight_end', 1.0/4.0)
toward('stint', 'longbow', 1.0/6.0)
v = pos['treacle'] - pos['taxumi']
pos['starlight_end'] = (pos['treacle']+pos['taxumi'])/2.0 + v.rotate(-90)/2.0*0.7

toward('ngc1317', 'stelman', -1.0/3.0)

pos['norn'] += (pos['pisces_prime']-pos['bonanza']) / 3.0
pos['reptile'] += (pos['newmarch']-pos['armorhead']) / 6.0

v = (pos['aesir']-pos['vanir']) / 4.0
pos['aesir'] += v
pos['vanir'] += v


# Anubis BH

pos['dohriabi'] += (pos['dohriabi']-pos['overture']) / 4.0
pos['anubis_black_hole'] += (pos['ngc13674']-pos['ngc1562']) / 8.0

v1 = pos['anubis_black_hole'] - pos['ngc1292']
v2 = pos['ngc5483'] + pos['ngc7078'] - pos['ngc1292']*2.0
t = v1.normalize() / v2.normalize()
for i in ['ngc5483', 'ngc7078', 'ngc4746']:
   pos[i] = (pos[i]-pos['ngc1292'])*t + pos['ngc1292']

v = (pos['octavian'] - pos['copernicus']) / 3.0
pos['copernicus'] += v
pos['octavian'] += v

l = (pos['ngc2601'] - pos['anubis_black_hole']).size()
v = (pos['ngc2601'] + pos['ngc11935'] - pos['anubis_black_hole']*2.0).normalize()
pos['zied'] = pos['anubis_black_hole'] + v*l

v1 = pos['ngc7078'] - pos['anubis_black_hole']
v2 = pos['octavian'] - pos['anubis_black_hole']
v = v1 + v2

v = v.normalize(((v1.size() + v2.size())/2.0))
pos['ngc7533'] = pos['anubis_black_hole'] + v

v = pos['ngc5483'] - pos['anubis_black_hole']
pos['ngc11935'] = pos['anubis_black_hole'] + (pos['ngc11935'] - pos['anubis_black_hole']).normalize(v.size())


# Thurion space

pos['nava'] = pos['flow'] + pos['vean'] - pos['aesria']

v = (pos['tempus']-pos['katami']) - (pos['aesria']-pos['flow'])
for i in ['tempus', 'aesria', 'flow', 'vean', 'nava']:
   pos[i] -= v


# Smoothen tradelane

tradelane = set()
for k in pos:
   if k[0] != '_':
      T = fil_ET(ssys_fil(k)).getroot()
      for e in T.findall('tags/tag'):
         if e.text == 'tradelane':
            tradelane.add(k)
            break

newp = dict()
for k in pos:
   if k[0] != '_' and (k in tradelane):
      tln = [s for (s, _) in ssysneigh(k) if (s in tradelane)]
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
      for n in [s for (s, _) in ssysneigh(k) if (s in tradelane)]:
         total += (pos[n]-pos[k]).size()
         count += 1
avg = total / count

for k in pos:
   if k[:3] == 'ngc' and k[3:] not in ['22375', '20489', '4746', '9415']:
      if len(n := ssysneigh(k)) == 1:
         n = n[0][0]
         v = pos[k] - pos[n]
         if v.size() < avg:
            pos[k] = pos[n] + v.normalize(avg)


# Position virtual systems

v = vec(avg, 0) * 0.4
for f, t, a in [
   ('beeklo',     'crimson_gauntlet',        120),
   ('anrique',    'test_of_renewal',         90),
   ('anarbalis',  'test_of_purification',    -60),
   ('churchill',  'test_of_alacrity',        0),
   ('ulysses',    'test_of_enlightenment',   135),
   ('aesir',      'test_of_devotion',        135),
]:
   pos[t] = pos[f] + v.rotate(a)


# Apply to ssys/

off = (pos['dohriabi'] - pos['anubis_black_hole']) / 2.0
for k in pos:
   pos[k] += off
   pos[k] = round(pos[k], 9)
   if k[0] != '_':
      nam = ssys_fil(k)
      o = fil_ET(nam)
      if (e := o.getroot().find('pos')) is not None:
         vec_to_element(e, pos[k])
      o.write(nam)
