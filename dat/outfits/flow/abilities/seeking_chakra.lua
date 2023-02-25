local flow = require "ships.lua.lib.flow"
local fmt = require "format"

function onadd( _p, po )
   local size = po:slot().size
   if size=="Small" then
      mem.flow_cost   = 25
      mem.ref         = outfit.get("Seeking Chakra Small")
      mem.cooldown    = 3
   else
      error(fmt.f(_("Flow ability '{outfit}' put into slot of unknown size '{size}'!"),
         {outfit=po:outfit(),size=size}))
   end
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
   else
      po:progress( mem.timer / mem.cooldown )
   end
end

function ontoggle( p, po, on )
   if not on then return false end

   if mem.timer > 0 then return false end

   local f = flow.get( p )
   if f < mem.flow_cost then
      return false
   end
   flow.dec( p, mem.flow_cost )

   local dir = p:dir()
   local sw, sh = p:ship():dims()
   local pos = p:pos()+vec2.newP( (sw+sh)*0.5+10, dir )
   po:munition( p, mem.ref, p:target(), dir, pos )
   mem.timer = mem.cooldown * p:shipstat("cooldown_mod",true)
   po:state("cooldown")
   po:progress(1)

   return true
end

function onimpact( _p, target )
   target:effectAdd("Chakra Corruption")
end
