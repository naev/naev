#!/usr/bin/env python3
#'input'  : 'core_engine/medium/nexus_arrow_700_engine.xml',
import helper as h
N_ = lambda text: text
data = h.read()

o = data['outfit']
o['@name'] = N_('Corsair Engine')

general = o['general']
del general['shortname']
general['unique'] = None
general['rarity'] = 6
general['price'] = 1e6
general['description'] = N_("""there once was a big mighty corsair
who was said to be the finest heir
of some noble great house
almost tied as a spouse
before they quickly took to the air""")
del general['slot']['@prop_extra']

specific = o['specific']
del specific['time_mod']
specific['jump_distance'] = { 'pri': 25 }
#specific['lua_inline'] = 'local set = require("outfits.lib.set")'
specific['lua_inline_post'] = 'require("outfits.core_sets.corsair_engine").init()'

data.save()
