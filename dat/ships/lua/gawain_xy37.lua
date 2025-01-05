local fmt = require "format"

function descextra( _p, _s )
   return "#b"..fmt.f(_("Gains action speed and lowers time constant the lower shields are, to a maximum of a {bonus}% bonus."), {bonus=50}).."#0"
end

function update( p, _dt )
   local s = p:shield()
   local bonus = (1-s) * 50
   local last = mem.last or 1
   if last ~= s then
      p:shippropSet{ -- Should overwrit
         ["time_speedup"]=bonus,
         ["time_mod"]=-bonus,
      }
      mem.last = s
   end
end
