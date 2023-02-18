local flow = require "ships.lua.lib.flow"

local flow_cost, ref
--local cooldown = 3

function onload( _o )
   -- TODO make outfit specific
   flow_cost = 25
   ref = outfit.get("Seeking Chakra Small")
end


-- This should trigger when the pilot is disabled or killed and destroy the
-- hologram if it is up
function ontoggle( p, po, on )
   if on then
      local f = flow.get( p )
      if f < flow_cost then
         return false
      end
      flow.dec( p, flow_cost )

      local dir = p:dir()
      for i=1,10 do
         local d = dir + math.pi*2.0/10*i
         po:munition( p, ref, p:target(), d )
      end

      return true
   end
   return false
end
