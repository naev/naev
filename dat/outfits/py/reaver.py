#!/usr/bin/env python3
import helper as h
N_ = lambda text: text
data = h.read()

data['outfit']['@name'] = N_('Reaver Cannon')

general = data['outfit']['general']
general['rarity'] = '2'
general['description'] = N_("The complex engineering specifications and difficulty of mass production doomed the Reaver Cannon from the start. However, in the right hands, given the extreme long range and high penetration for a weapon of its size, it can be very deadly.")
h.add_i(general, 'price', 130e3 )
h.add_i(general, 'mass', 2 )
h.mul_i(general, 'cpu', 1.1 )

specific = data['outfit']['specific']
h.mul_f(specific, 'range', 1.5 )
h.mul_f(specific, 'falloff', 1.2 )
h.mul_f(specific, 'delay', 2 )
h.mul_f(specific, 'energy', 2.0 )
h.mul_f(specific, 'swivel', 1.5 )
h.mul_i(specific, 'trackmax', 0.8333333333 )

damage = specific['damage']
h.mul_f(damage, 'physical', 2.0 )
h.mul_i(damage, 'penetrate', 1.3 )

h.write( data )
