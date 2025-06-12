#!/usr/bin/env python3
import helper as h
N_ = lambda text: text
data = h.read()

o = data['outfit']
o['@name'] = N_('Corsair Systems')

general = o['general']
del general['shortname']
general['unique'] = None
general['rarity'] = 6
general['price'] = 1e6
general['description'] = "TODO"
del general['slot']['@prop_extra']

specific = o['specific']
ref = h.get_outfit_dict( h.INPUT, True )
del ref['cooldown_time']
ref['ew_detect'] = (ref['ew_detect'][0]+5.0,)
lua = f"""
local set = require("outfits.lib.set")
{h.to_multicore_lua( ref, True, "set.set" )}
require("outfits.core_sets.corsair_systems").init( multicore, set )
"""
specific['lua_inline'] = lua

h.write( data )
