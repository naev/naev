#!/usr/bin/env python3
import helper as h
N_ = lambda text: text
data = h.read()

data['outfit']['@name'] = N_('Reaver Cannon')

general = data['outfit']['general']
general['rarity'] = '2'
general['description'] = N_("TODO")
h.add_i(general, 'price', 130e3 )
h.add_i(general, 'mass', -1 )
h.mul_i(general, 'cpu', 1.1 )

specific = data['outfit']['specific']
h.mul_f(specific, 'range', 1.5 )
h.mul_f(specific, 'falloff', 1.2 )
h.mul_f(specific, 'delay', 2.5 )
h.mul_f(specific, 'energy', 2.0 )
h.mul_f(specific, 'swivel', 1.5 )
h.mul_i(specific, 'trackmax', 0.8333333333 )

damage = specific['damage']
h.mul_f(damage, 'physical', 3.0 )
h.mul_i(damage, 'penetrate', 1.5 )

h.write( data )
