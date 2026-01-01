#!/usr/bin/env python3

import helper as h
N_ = lambda text: text
Add = lambda y: (lambda x: x + y)
Mul = lambda y: (lambda x: x * y)
data = h.read()

data['outfit']['@name'] = N_('Neutralizer')

data['outfit']['general'] |= {
   'rarity': 2,
   'description': N_("Based on the Heavy Ion Cannon, the Neutralizer is proof to the importance of specialized engineering. With most of the hardware and software significantly modified or replaced, the Neutralizer shows significant improvements in nearly all facets. Due to the fact that few were made, it is a fairly rare find, but an import asset in a pilot's arsenal."),
   '$price': Add(150e3),
   '$mass': Add(-2),
   '$cpu': lambda x: round(x * 0.92),
}

specific = data['outfit']['specific']
specific |= {
   '$range': Mul(1.1),
   '$falloff': Mul(1.2),
   '$delay': Mul(1.05),
   '$energy': Mul(1.05),
   '$swivel': Mul(1.5),
   '$trackmax': lambda x: round(x * 0.8333333333),
   'lua': 'outfits/lib/matrix_sell.lua',
}

specific['damage'] |= {
   '$disable': Mul(1.2),
   '$physical': Mul(1.25),
   '$penetrate': lambda x: round(x * 1.15),
}

data.save()
