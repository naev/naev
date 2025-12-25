local fmt = require "format"

local MAXBONUS = 30 / 100

function descextra( _p, _s )
   return "#b"..fmt.f(_("Gains action speed and lowers time constant the lower shields are, to a maximum of a {bonus}% bonus."), {bonus=MAXBONUS*100}).."#0"
end

function update( p )
   local s = p:shield()
   local bonus = (100-s) * MAXBONUS
   local last = mem.last or 1
   if last ~= s then
      p:shippropSet{ -- Should overwrite existing values
         ["action_speed"]=bonus,
         ["time_mod"]=-bonus,
      }
      mem.last = s
   end
end
