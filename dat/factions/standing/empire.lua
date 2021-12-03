-- Empire faction standing script
local sbase = require "factions.standing.lib.base"

standing = sbase.newStanding{
   fct            = faction.get("Empire"),
   cap_kill       = 15,
   delta_distress = {-1, 0},    -- Maximum change constraints
   delta_kill     = {-5, 1},    -- Maximum change constraints
   cap_misn_init  = 30,
   cap_misn_var = "_fcap_empire",
}

local sec_hit_min = 10

function standing:hit( current, amount, source, secondary )
   local start_standing = self.fct:playerStanding()
   local f = sbase.Standing.hit( self, current, amount, source, secondary )
   if secondary and amount < 0 and f < sec_hit_min then
      f = math.min( start_standing, sec_hit_min )
   end
   return f
end
