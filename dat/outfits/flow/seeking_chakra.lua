local flow = require "ships.lua.lib.flow"

local flow_cost, ref
local cooldown = 3

function onload( _o )
   -- TODO make outfit specific
   flow_cost = 25
   ref = outfit.get("Seeking Chakra Small")
end

function init( _p, po )
   mem.timer = 0
   po:state("off")
end

function update( _p, po, dt )
   if mem.timer < 0 then return end

   mem.timer = mem.timer - dt
   if mem.timer < 0 then
      po:state("off")
   end
end

-- This should trigger when the pilot is disabled or killed and destroy the
-- hologram if it is up
function ontoggle( p, po, on )
   if on then
      if mem.timer > 0 then
         return
      end

      local f = flow.get( p )
      if f < flow_cost then
         return false
      end
      flow.dec( p, flow_cost )

      local dir = p:dir()
      local sw, sh = p:ship():dims()
      local pos = p:pos()+vec2.newP( (sw+sh)*0.5+10, dir )
      po:munition( p, ref, p:target(), dir, pos )
      mem.timer = cooldown
      po:state("cooldown")
      po:progress(1)

      return true
   end
   return false
end
