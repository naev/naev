#!/usr/bin/env python3

import helper as h
N_ = lambda text: text
data = h.read()

data['outfit']['@name'] = N_('Reaver Cannon')

general = data['outfit']['general']
general['rarity'] = '2'
general['description'] = N_('The complex engineering specifications and difficulty of mass production doomed the Reaver Cannon from the start. However, in the right hands, given the extreme long range and high penetration for a weapon of its size, it can be very deadly.')
general['$price'] += 130e3
general['$mass'] += 2
general['$cpu'] = round(general['$cpu'] * 1.1)

specific = data['outfit']['specific']
specific['$range'] *= 1.5
specific['$falloff'] *= 1.2
specific['$delay'] *= 2
specific['$energy'] *= 2.0
specific['$swivel'] *= 1.5
specific['$trackmax'] = round(specific['$trackmax'] * 0.8333333333)
specific['lua'] = 'outfits/lib/matrix_sell.lua'
specific['gfx'] = {
   '@type' : 'shader',
   '@size' : '10',
   '@col_size' : '8',
   '#text' : 'particles/reaver.frag',
}

damage = specific['damage']
damage['$physical'] *= 2
damage['$penetrate'] = round(damage['$penetrate'] * 1.3)

data.save()
