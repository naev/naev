#!/usr/bin/env python3
import helper as h
N_ = lambda text: text
data = h.read()

o = data['outfit']
o['@name'] = N_('Junker Plates')

general = o['general']
#del general['shortname']
general['unique'] = None
general['rarity'] = 6
general['$price'] = 200e3
general['description'] = N_("TODO")

specific = o['specific']
specific['mass'] = str( int(specific['mass']) + 5 )
specific['armour']['$pri'] += 20
specific['armour']['$sec'] += 10
specific['cargo_mod']['$pri'] -= 15
specific['cargo_mod']['$sec'] -= 5
specific['lua_inline_post'] = "require('outfits.lib.sets.junker').init( true )"

data.save()
