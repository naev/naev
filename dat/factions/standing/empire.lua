-- Empire faction standing script
local sbase = require "factions.standing.lib.base"
standing = sbase.newStanding( require("factions.empire") )

local sec_hit_min = 10

function standing:hit( current, amount, source, secondary )
   local start_standing = self.fct:playerStanding()
   local f = sbase.Standing.hit( self, current, amount, source, secondary )
   if secondary and amount < 0 and f < sec_hit_min then
      f = math.min( start_standing, sec_hit_min )
   end
   return f
end
