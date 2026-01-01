#!/usr/bin/env python3

import helper as h
Add = lambda y: (lambda x: x + y)
Mul = lambda y: (lambda x: x * y)
N_ = lambda text: text
data = h.read()

data['outfit']['@name'] = N_('Reaver Cannon')

general = data['outfit']['general']
general |= {
   'rarity': '2',
   'description': N_('The complex engineering specifications and difficulty of mass production doomed the Reaver Cannon from the start. However, in the right hands, given the extreme long range and high penetration for a weapon of its size, it can be very deadly.'),
   '$price': Add(130e3),
   '$mass': Add(2),
   '$cpu': lambda x: round(x * 1.1)
}

specific = data['outfit']['specific']
specific |= {
   '$range': Mul(1.5),
   '$falloff': Mul(1.2),
   '$delay': Mul(2),
   '$energy': Mul(2.0),
   '$swivel': Mul(1.5),
   '$trackmax': lambda x: round(x * 0.8333333333),
   'lua': 'outfits/lib/matrix_sell.lua',
}

specific['gfx'] = {
   '@type': 'shader',
   '@size': '10',
   '@col_size' : '8',
   '#text': 'particles/reaver.frag',
}

specific['damage'] |= {
   '$physical': Mul(2),
   '$penetrate': lambda x: round(x * 1.3),
}

data.save()
