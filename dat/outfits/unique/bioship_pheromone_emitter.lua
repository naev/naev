local audio = require 'love.audio'
local luaspfx = require 'luaspfx'

local cooldown = 15 -- cooldown time in seconds

local sfx = audio.newSource( 'snd/sounds/ping.ogg' )

function init( p, po )
   mem.timer = nil
   po:state("off")
   mem.isp = (p == player.pilot())
end

function update( _p, po, dt )
   if not mem.timer then return end

   mem.timer = mem.timer - dt
   po:progress( mem.timer / cooldown )
   if mem.timer < 0 then
      po:state("off")
      mem.timer = nil
   end
end

function ontoggle( p, po, _on )
   -- Still on cooldown
   if mem.timer and mem.timer > 0 then
      return false
   end

   -- Visual effect
   local ppos = p:pos()
   luaspfx.pheromones( ppos, p:vel() )
   if mem.isp then
      luaspfx.sfx( true, nil, sfx )

      -- Can trigger scan hooks
      naev.trigger( "bioship_pheromones" )
   else
      luaspfx.sfx( ppos, p:vel(), sfx )
   end

   po:state("cooldown")
   po:progress(1)
   mem.timer = cooldown * p:shipstat("cooldown_mod",true)
   return true
end
