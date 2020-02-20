

include "dat/factions/standing/skel.lua"


_fcap_kill     = 0 -- Kill cap
_fdelta_distress = {-1.5, 0} -- Maximum change constraints
_fdelta_kill     = {-7, 2} -- Maximum change constraints
_fcap_misn     = 30 -- Starting mission cap, gets overwritten
_fcap_misn_var = "_fcap_miner"
_fthis         = faction.get("Miner")


function faction_hit( current, amount, source, secondary )
    return default_hit(current, amount, source, secondary)
end
