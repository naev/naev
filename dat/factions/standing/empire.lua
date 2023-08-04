-- Empire faction standing script
local sbase = require "factions.standing.lib.base"
sbase.init( require("factions.empire") )

local sec_hit_min = 10

local oldhit = hit
function hit( current, amount, source, secondary )
   local start_standing = sbase.fct:playerStanding()
   local f = oldhit( current, amount, source, secondary )
   if secondary and amount < 0 and f < sec_hit_min then
      f = math.min( start_standing, sec_hit_min )
   end
   return f
end
