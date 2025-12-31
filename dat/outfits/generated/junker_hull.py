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
general['description'] = N_("An assortment of different chunks of scavenged ship hulls linked together to form a surprisingly durable hull plating. It comes with a surprising amount of empty area to fit cargo, and the unorthodox design makes it a bit hard for weapon systems to lock onto.")

specific = o['specific']
specific['mass'] = str( int(specific['mass']) + 10 )
specific['armour']['$pri'] += 10
specific['armour']['$sec'] += 5
specific['absorb']['$pri'] += 1
specific['cargo_mod']['$pri'] -= 10
specific['cargo_mod']['$sec'] -= 5
specific['cargo_inertia']['$pri'] += 10
specific['cargo_inertia']['$sec'] += 5
specific['ew_signature'] = { '$pri': -9, '$sec': -5 }
specific['jam_chance'] = { '$pri': 9, '$sec': 5 }
specific['lua_inline_post'] = "require('outfits.lib.sets.junker').init( true )"

data.save()
