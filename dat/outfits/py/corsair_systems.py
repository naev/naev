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
setname = N_("Corsair")
desc1 = N_("TODO")
lua = f"""
local set = require("outfits.lib.set")
{h.to_multicore_lua( ref, True, "set.set" )}
set.init( _("{setname}"),
    {{ outfit.get("Corsair Systems"), }},
    {{
    [1] = {{
        desc = _("{desc1}"),
        func = function ( p, po, on )
            print( p, po, on )
        end,
    }},
    }},
    true
)
"""
specific['lua_inline'] = lua

h.write( data )
