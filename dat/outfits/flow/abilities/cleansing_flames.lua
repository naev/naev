local flow = require "ships.lua.lib.flow"

local flow_cost, ref, range
local cooldown

function onload( o )
   if o==outfit.get("Lesser Cleansing Flames") then
      flow_cost   = 40
      ref         = nil
      range       = 200
      cooldown    = 5
   else
      error(_("Unknown outfit using script!"))
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
      po:progress( mem.timer / cooldown )
   end
end

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

      -- TODO spfx + damage stuff
      print( ref, range )

      mem.timer = cooldown * p:shipstat("cooldown_mod",true)
      po:state("cooldown")
      po:progress(1)

      return true
   end
   return false
end
