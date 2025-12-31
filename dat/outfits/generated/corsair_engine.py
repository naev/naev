#!/usr/bin/env python3

import helper as h
N_ = lambda text: text
data = h.read()

o = data['outfit']
o['@name'] = N_('Corsair Engine')

general = o['general']
general['shortname']= None
general['unique'] = ''
general['rarity'] = 6
general['$price'] = 1e6
general['description'] = N_("""there once was a big mighty corsair
who was said to be the finest heir
of some noble great house
almost tied as a spouse
before they quickly took to the air""")
general['slot']['@prop_extra']= None

specific = o['specific']
specific['time_mod']= None
specific['jump_distance'] = 25
specific['lua_inline_post'] = "require('outfits.core_sets.corsair_engine').init()"

data.prisec_only(sec= False)
data.save()
