--[[
   basically just copied skel.lua and made it work.
--]]

include "dat/factions/standing/skel.lua"

-- Faction caps.
_fcap_kill       = 20 -- Kill cap
_fdelta_distress = {-1, 0} -- Maximum change constraints
_fdelta_kill     = {-5, 1} -- Maximum change constraints
_fcap_misn       = 100 -- Starting mission cap, gets overwritten
_fcap_misn_var   = "_fcap_nasin" -- Mission variable to use for limits
_fcap_mod_sec    = 0.3 -- Modulation from secondary
_fthis           = faction.get("Nasin")

function faction_hit( current, amount, source, secondary )
    return default_hit(current, amount, source, secondary)
end
