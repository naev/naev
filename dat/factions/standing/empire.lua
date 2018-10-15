

include "dat/factions/standing/skel.lua"


_fcap_kill     = 15 -- Kill cap
_fdelta_distress = {-1, 0} -- Maximum change constraints
_fdelta_kill     = {-5, 1} -- Maximum change constraints
_fcap_misn     = 30 -- Starting mission cap, gets overwritten
_fcap_misn_var = "_fcap_empire"

_fthis         = faction.get("Empire")

sec_hit_min = 10


function faction_hit( current, amount, source, secondary )
   local start_standing = _fthis:playerStanding()
   local f = default_hit( current, amount, source, secondary )
   if ( secondary and amount < 0 and f < sec_hit_min ) then
      f = math.min( start_standing, sec_hit_min )
   end
   return f
end
