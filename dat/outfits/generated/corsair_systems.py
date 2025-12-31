#!/usr/bin/env python3

import helper as h
N_ = lambda text: text
data = h.read()

o = data['outfit']
o['@name'] = N_('Corsair Systems')

o['general'].update({
   'shortname': None,
   'unique': '',
   'rarity': 6,
   '$price': 1e6,
   'description': N_("""oh, the ferocious mighty corsair
never sitting in their captain's chair
least they puke out their grub
all over the ship hub
in space is it also mal de mer?"""),
})

o['general']['slot']['@prop_extra']= None

o['specific'].update({
   'cooldown_time': None,
   'ew_hide': -15,
   'lua_inline_post': "require('outfits.core_sets.corsair_systems').init()",
})

o['specific']['ew_detect']['$pri'] += 5
data.prisec_only(sec= False)
data.save()
