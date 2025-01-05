local fmt = require "format"

update_dt = 1 -- Run once per second

local BONUS_INC = 10
local BONUS_MAX = 70

function descextra( _p )
   return "#o"..fmt.f(_("Shooting any weapon increases fire rate by {inc}%, to a maximum of {max}%. This effect can only be triggered once per second."),
      {inc=BONUS_INC, max=BONUS_MAX}).."#0"
end

function init( _p )
   mem.bonus = 0
   mem.shot = false
end

function update( p )
   if mem.shot then
      mem.shot = false
   elseif mem.bonus > 0 then
      p:shippropReset()
      mem.bonus = 0
   end
end

function onshootany( p )
   if not mem.shot then
      mem.bonus = math.min( mem.bonus + BONUS_INC, BONUS_MAX )
      p:shippropSet( "fwd_firerate", mem.bonus )
      mem.shot = true
   end
end
