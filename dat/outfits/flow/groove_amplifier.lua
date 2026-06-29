notactive = true -- Doesn't become active

local flow = require "ships.lua.lib.flow"
local fmt = require "format"

function descextra( p, o )
   local oname = o:nameRaw()
   local amount, regen
   if oname=="Internal Groove Synthesizer" then
      if p then
         amount = flow.list_base[p:ship():nameRaw()]
         regen  = flow.list_regen[p:ship():nameRaw()]
      else
         amount = _("??")
         regen  = _("??")
      end
   else
      amount = flow.list_base[o:nameRaw()]
      regen  = flow.list_regen[o:nameRaw()]
   end
   return fmt.f("#y".._("Provides {flow} maximum flow capacity, {regen} flow regeneration per second up to {capacity}% maximum capacity, and allows a ship to use flow and allows gaining {amount}% of damage done as flow.").."#0", {
      flow     = amount,
      regen    = regen,
      capacity = 10,
      amount   = 33,
   })
end

function init( p )
   flow.recalculate( p )
end

function update( p, _po, dt )
   flow.update( p, dt )
end

-- Done onanyimpact and not onhit
function onanyimpact( p, _po, _target, _pos, _vel, _o, armour, shield, _disable )
   flow.inc( p, 0.33*(armour+shield) )
end

function onremove( p, _po )
   -- Can't use flow.recalculate() because this is run while the outfit is
   -- still equipped. So we just disable the internals.
   local sm = p:shipMemory()
   sm._flow_mod = nil
   sm._flow_base = nil
   sm._flow_regen = nil
end
