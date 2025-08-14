#!/usr/bin/env python3
import helper as h
N_ = lambda text: text
data = h.read()

o = data['outfit']
o['@name'] = N_('Corsair Hull Plating')

general = o['general']
del general['shortname']
general['unique'] = None
general['rarity'] = 6
general['$price'] = 1e6
general['description'] = N_("""wish I be like that lovely corsair
the prettiest of all pirate hair
be drinking at the bar
eyes green like fluorspar
irresistible buccaneer stare""")
del general['slot']['@prop_extra']

specific = o['specific']
specific['absorb']['$pri'] -= 4
specific['ew_stealth_timer'] = -10
specific['jam_chance'] = 18
specific['lua_inline_post'] = "require('outfits.core_sets.corsair_hull').init()"

data.prisec_only(sec= False)
data.save()
