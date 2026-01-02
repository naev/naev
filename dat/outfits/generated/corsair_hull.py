#!/usr/bin/env python3

import helper as h
N_ = lambda text: text
Rem = h.Rem
data = h.read()
data['outfit'] ^= {
   '@name': N_('Corsair Hull Plating'),
   'general': {
      'shortname': Rem,
      'unique': None,
      'rarity': 6,
      '$price': 1e6,
      'description': N_("""wish I be like that lovely corsair
the prettiest of all pirate hair
be drinking at the bar
eyes green like fluorspar
irresistible buccaneer stare"""),
      'slot': {'@prop_extra': Rem},
   },
   'specific': {
      'ew_stealth_timer': -10,
      'jam_chance': 18,
      'lua_inline_post': "require('outfits.core_sets.corsair_hull').init()",
      'absorb': {'$pri': lambda x: x - 4},
   },
}
data.prisec_only(sec= False)
data.save()
