#!/usr/bin/env python3
import helper as h
N_ = lambda text: text
data = h.read()

o = data['outfit']
o['@name'] = N_('Corsair Engine')

general = o['general']
del general['shortname']
general['unique'] = None
general['rarity'] = 6
general['price'] = 1e6
general['description'] = "TODO"
del general['slot']['@prop_extra']

specific = o['specific']
ref = h.get_outfit_dict( h.INPUT, True )
del ref['time_mod']
ref['jump_distance'] = (25,)
lua = f"""
local set = require("outfits.lib.set")
{h.to_multicore_lua( ref, True, "set.set" )}
require("outfits.core_sets.corsair_engine").init( multicore, set )
"""
specific['lua_inline'] = lua

h.write( data )
