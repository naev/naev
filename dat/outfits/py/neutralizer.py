#!/usr/bin/env python3
import helper as h
N_ = lambda text: text
data = h.read()

data['outfit']['@name'] = N_('Neutralizer')

general = data['outfit']['general']
general['rarity'] = '2'
general['description'] = N_("Based on the Heavy Ion Cannon, the Neutralizer is proof to the importance of specialized engineering. With most of the hardware and software significantly modified or replaced, the Neutralizer shows significant improvements in nearly all facets. Due to the fact that few were made, it is a fairly rare find, but an import asset in a pilot's arsenal.")
h.add_i(general, 'price', 150e3 )
h.add_i(general, 'mass', -2 )
h.mul_i(general, 'cpu', 0.92 )

specific = data['outfit']['specific']
h.mul_f(specific, 'range', 1.1 )
h.mul_f(specific, 'falloff', 1.2 )
h.mul_f(specific, 'delay', 1.05 )
h.mul_f(specific, 'energy', 1.05 )
h.mul_f(specific, 'swivel', 1.5 )
h.mul_i(specific, 'trackmax', 0.8333333333 )

damage = specific['damage']
h.mul_f(damage, 'disable', 1.2 )
h.mul_f(damage, 'physical', 1.25 )
h.mul_i(damage, 'penetrate', 1.15 )

h.write( data )
