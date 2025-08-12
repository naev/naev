local fmt = require "format"

local active = 4 -- active time in seconds
local cooldown = 15 -- cooldown time in seconds
local boost = 5 -- How much the shield regen is increased by
local efficiency = 3 -- GJ of energy used per shield recovered

function descextra( _p, _o, _po )
   return fmt.f(_("Increases shield regeneration by {boost}%. Active for {active} seconds with a cooldown of {cooldown} seconds. Uses {efficiency} GJ of energy for each GJ of shield recovered."),{
      boost=boost*100,
      active=active,
      cooldown=cooldown,
      efficiency=efficiency,
   })
end

local function turnon( p, po )
   -- Still on cooldown
   if mem.timer > 0 then
      return false
   end
   local _a, s = p:health()
   if s > 99 then
      return false -- Don't activate at full health
   end
   po:state("on")
   po:progress(1)
   mem.active = true

   -- Apply effect
   -- We would want to recompute this effect every update, but this leads to
   -- the effect affecting itself and going to near infinity
   local ps = p:stats()
   local regen = boost * ps.shield_regen
   po:set( "shield_regen", regen )
   po:set( "shielddown_mod", -98 )
   po:set( "energy_regen_malus", efficiency * regen )

   mem.timer = active

   -- apply nice shader effect
   p:effectAdd("Shield Boost")

   return true
end

local function turnoff( p, po )
   if not mem.active then
      return false
   end
   po:state("cooldown")
   po:progress(1)
   po:clear() -- clear stat modifications
   mem.timer = cooldown * p:shipstat("cooldown_mod",true)
   mem.active = false
   p:effectAdd("Shield Boost", 1)
   return true
end

function init( p, po )
   turnoff( p, po )
   mem.timer = 0
   po:state("off")
end

function update( p, po, dt )
   mem.timer = mem.timer - dt
   if mem.active then
      po:progress( mem.timer / active )
      local _a, s = p:health()
      if mem.timer < 0 or s > 99 then
         turnoff( p, po )
      end
   else
      po:progress( mem.timer / cooldown )
      if mem.timer < 0 then
         po:state("off")
      end
   end
end

function ontoggle( p, po, on )
   if on then
      return turnon( p, po )
   else
      return turnoff( p, po )
   end
end
