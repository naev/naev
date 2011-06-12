

include "dat/factions/skel.lua"


_fcap_distress = 5 -- Distress cap
_fcap_kill     = 5 -- Kill cap
_fdelta_distress = {-0.5, 0} -- Maximum change constraints
_fdelta_kill     = {-5, 1} -- Maximum change constraints
_fcap_misn     = 30 -- Starting mission cap, gets overwritten
_fcap_misn_var = "_fcap_proteron"
_fthis         = faction.get("Proteron")


