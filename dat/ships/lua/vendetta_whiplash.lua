local fmt = require "format"

update_dt = 1 -- Run once per second

local BONUS_INC = 20
local BONUS_MAX = 80

function descextra( _p )
   return "#o"..fmt.f(_("Shooting any weapon increases fire rate by {inc}% each second, to a maximum of {max}%."),
      {inc=BONUS_INC, max=BONUS_MAX}).."#0"
end

function init( _p )
   mem.bonus = 0
   mem.shot = false
end

function update( p )
   if mem.shot then
      mem.shot = false
   elseif mem.bonus >= 0 then
      -- Be nice and ramp down, although twice as fast
      mem.bonus = mem.bonus - 2 * BONUS_INC
      if mem.bonus > 0 then
         p:shippropSet( "weapon_firerate", mem.bonus )
      else
         p:shippropReset()
         mem.bonus = -10 -- we start at -10 or it'll always start at 1 stack
      end
   end
end

function onshootany( p )
   if not mem.shot then
      mem.bonus = math.min( mem.bonus + BONUS_INC, BONUS_MAX )
      p:shippropSet( "weapon_firerate", mem.bonus )
      mem.shot = true
   end
end
