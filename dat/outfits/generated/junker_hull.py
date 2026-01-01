#!/usr/bin/env python3

import helper as h
N_ = lambda text: text
Add = lambda y: (lambda x: x + y)
data = h.read()

o = data['outfit']
o['@name'] = N_('Junker Plates')

o['general'] |= {
   'shortname': Rem,
   'unique': None,
   'rarity': 6,
   '$price': 200e3,
   'description': N_("An assortment of different chunks of scavenged ship hulls linked together to form a surprisingly durable hull plating. It comes with a surprising amount of empty area to fit cargo, and the unorthodox design makes it a bit hard for weapon systems to lock onto."),
}

specific = o['specific']
specific |= {
   '$mass': Add(10),
   'ew_signature': { '$pri': -9, '$sec': -5 },
   'jam_chance': { '$pri': 9, '$sec': 5 },
   'lua_inline_post': "require('outfits.lib.sets.junker').init( true )",
}
specific['armour'] |= {'$pri': Add(10), '$sec': Add(5)}
specific['absorb']['$pri'] = Add(1)
specific['cargo_mod'] |= {'$pri': Add(-10), '$sec': Add(-5)}
specific['cargo_inertia'] |= {'$pri': Add(10), '$sec': Add(5)}

data.save()
