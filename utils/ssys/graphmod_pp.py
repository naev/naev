#!/usr/bin/env python3


from sys import stderr, argv, exit


if __name__ != '__main__':
   raise Exception('This module is only intended to be used as main.')

if argv[1:] != []:
   stderr.write('usage: ' + argv[0].split('/')[-1] + '\n')
   stderr.write('  Reads a graph file on stdin, outputs a graph on stdout.\n')
   stderr.write('  Intended as a postprocessing for neato output.\n')
   exit(0)

from geometry import bb, vec, segment
from graphmod import sys_pos as pos

from ssys_graph import twins as small
for (i,j), q in small.items():
   a = pos[i]*q + pos[j]*(1.0-q)
   pos[i] = (pos[i]+a) / 2.0
   pos[j] = (pos[j]+a) / 2.0


pos['syndania'] = vec(pos['syndania'][0], pos['stint'][1])

Spir = ['syndania', 'nirtos', 'sagittarius', 'hopa', 'scholzs_star',
   'veses', 'alpha_centauri', 'padonia']

Scenter = (pos[Spir[0]] + pos[Spir[4]]) / 2.0
v = (pos[Spir[0]] - Scenter) * 0.75
v = v.rotate(-50)
Scenter = (pos['scholzs_star'] + Scenter) / 2.0 + v.size()/4.0*vec(0,1)
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

def toward( src, dst, q ):
   global pos
   pos[src] += (pos[dst]-pos[src]) * q

toward('syndania', 'jommel', 1.0/4.0)
#toward('scholzs_star', 'haered', 1.0/4.0)

length = (pos['haered']-pos['cleai']).size()
haered = pos['haered']
pos['haered'] = pos['cleai'] + (pos['haered']-pos['cleai']).rotate(60)

v = pos['hystera'] - pos['leporis']
v = v.rotate(-60)
pos['leporis'] = pos['haered'] + (pos['leporis']-haered).normalize(v.size())
pos['hystera'] = pos['leporis'] + v

u = v.rotate(-60)
pos['korifa'] = pos['hystera'] + u
pos['apik'] = pos['leporis'] + u
pos['telika'] = pos['apik'] - v
pos['mida'] = pos['apik'] + u
pos['ekta'] = pos['mida'] - v
pos['akra'] = pos['mida'] + u

L = segment(pos['haered'], pos['south_bell']).line()
pos['cleai'] += (L - pos['cleai'])
L = segment(pos['cleai'], pos['south_bell']).line()
for s in ['maron', 'machea']:
   # mirror it across L
   pos[s] = pos[s] + 2.0*(L - pos[s])

pos['machea'] = pos['maron'] + (pos['machea']-pos['maron']).rotate(30)
pos['cleai'] = (pos['haered']+pos['maron']) / 2.0

proteron = ['leporis', 'hystera', 'korifa', 'apik', 'telika', 'mida', 'ekta', 'akra']
for s in proteron:
   pos[s] = pos['haered'] + (pos[s]-pos['haered']).rotate(-30)

u = pos['haered']-pos['cleai']
v = u.rotate(-75)-u
for s in proteron + ['haered', 'cleai']:
   pos[s] += v

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


# Increase terminal ngc dist to neighbour to at least average edge length.

from ssys_graph import xml_files_to_graph
_V, _pos, E, tradelane, _color = xml_files_to_graph()

total = 0.0
count = 0
for k in pos:
   if k[0] != '_':
      for n in [s for (s, _) in E[k] if (s in tradelane)]:
         total += (pos[n]-pos[k]).size()
         count += 1
avg = total / count

for k in pos:
   if k[:3] == 'ngc' and k[3:] not in ['22375', '20489', '4746', '9415']:
      if len(n := E[k]) == 1:
         n = n[0][0]
         v = pos[k] - pos[n]
         if v.size() < avg:
            pos[k] = pos[n] + v.normalize(avg)


# Qorel Corridor

pos['firk'] = (pos['gamma_polaris'] + pos['shakar'] + pos['apez']) / 3.0
pos['effetey'] = (pos['majesteka'] + pos['trohem']) / 2.0


off = (pos['dohriabi'] - pos['anubis_black_hole']) / 2.0
for k, v in pos.items():
   pos[k] += off
