#!/usr/bin/env python3

import helper as h
N_ = lambda text: text
Rem=h.Rem
data = h.read()

o = data['outfit']
o['@name'] = N_('Corsair Engine')
o['general'].update({
   'shortname': Rem,
   'unique': None,
   'rarity': 6,
   '$price': 1e6,
   'description': N_("""there once was a big mighty corsair
who was said to be the finest heir
of some noble great house
almost tied as a spouse
before they quickly took to the air"""),
})
o['general']['slot']['@prop_extra']= Rem
o['specific'].update({
   'time_mod': Rem,
   'jump_distance': 25,
   'lua_inline_post': "require('outfits.core_sets.corsair_engine').init()",
})

data.prisec_only(sec= False)
data.save()
