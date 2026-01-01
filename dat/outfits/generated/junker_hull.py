#!/usr/bin/env python3

import helper as h
N_ = lambda text: text
Add = lambda y: (lambda x: x + y)
data = h.read()
data['outfit'] ^= {
   '@name': N_('Junker Plates'),
   'general': {
      'shortname': Rem,
      'unique': None,
      'rarity': 6,
      '$price': 200e3,
      'description': N_("An assortment of different chunks of scavenged ship hulls linked together to form a surprisingly durable hull plating. It comes with a surprising amount of empty area to fit cargo, and the unorthodox design makes it a bit hard for weapon systems to lock onto."),
   },
   'specific': {
      '$mass': Add(10),
      'ew_signature': { '$pri': -9, '$sec': -5 },
      'jam_chance': { '$pri': 9, '$sec': 5 },
      'lua_inline_post': "require('outfits.lib.sets.junker').init( true )",
      'armour': {'$pri': Add(10), '$sec': Add(5)},
      'absorb': {'$pri': Add(1)},
      'cargo_mod': {'$pri': Add(-10), '$sec': Add(-5)},
      'cargo_inertia': {'$pri': Add(10), '$sec': Add(5)},
   },
}
data.save()
