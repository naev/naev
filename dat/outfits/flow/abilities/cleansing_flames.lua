local flow = require "ships.lua.lib.flow"
local fmt = require "format"

function onadd( _p, po )
   local size = po:slot().size
   if size=="Small" then
      mem.flow_cost   = 40
      mem.ref         = nil
      mem.range       = 200
      mem.cooldown    = 5
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
   if on then
      if mem.timer > 0 then
         return
      end

      local f = flow.get( p )
      if f < mem.flow_cost then
         return false
      end
      flow.dec( p, mem.flow_cost )

      -- TODO spfx + damage stuff
      print( mem.ref, mem.range )

      mem.timer = mem.cooldown * p:shipstat("cooldown_mod",true)
      po:state("cooldown")
      po:progress(1)

      return true
   end
   return false
end
