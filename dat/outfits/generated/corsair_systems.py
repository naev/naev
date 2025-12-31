#!/usr/bin/env python3

import helper as h
N_ = lambda text: text
data = h.read()

o = data['outfit']
o['@name'] = N_('Corsair Systems')

general = o['general']
general['shortname']= None
general['unique'] = ''
general['rarity'] = 6
general['$price'] = 1e6
general['description'] = N_("""oh, the ferocious mighty corsair
never sitting in their captain's chair
least they puke out their grub
all over the ship hub
in space is it also mal de mer?""")
general['slot']['@prop_extra']= None

specific = o['specific']
specific['cooldown_time']= None
specific['ew_detect']['$pri'] += 5
specific['ew_hide'] = -15
specific['lua_inline_post'] = "require('outfits.core_sets.corsair_systems').init()"

data.prisec_only(sec= False)
data.save()
