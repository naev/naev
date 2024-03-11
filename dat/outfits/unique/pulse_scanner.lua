local audio = require 'love.audio'
local luaspfx = require 'luaspfx'

local active = 10 -- active time in seconds
local cooldown = 15 -- cooldown time in seconds
local alertrange = 3e3 -- Distance to alert

local sfx = audio.newSource( 'snd/sounds/ping.ogg' )

local function turnon( p, po )
   -- Still on cooldown
   if mem.timer and mem.timer > 0 then
      return false
   end
   po:state("on")
   po:progress(1)
   mem.timer = active
   mem.active = true

   -- Visual effect
   local ppos = p:pos()
   luaspfx.pulse( ppos, p:vel() )
   if mem.isp then
      luaspfx.sfx( true, nil, sfx )

      -- Can trigger scan hooks
      naev.trigger( "poi_scan" )
   else
      luaspfx.sfx( ppos, p:vel(), sfx )
   end

   -- Will notify nearby ships
   for k,s in ipairs(pilot.getInrange( ppos, alertrange )) do
      pilot.msg( nil, s, "signal", ppos )
   end

   return true
end

local function turnoff( p, po )
   if not mem.active then
      return false
   end
   po:state("cooldown")
   po:progress(1)
   mem.timer = cooldown * p:shipstat("cooldown_mod",true)
   mem.active = false
   return true
end

function init( p, po )
   turnoff( p, po )
   mem.timer = nil
   po:state("off")
   mem.isp = (p == player.pilot())
end

function update( p, po, dt )
   if not mem.timer then return end

   mem.timer = mem.timer - dt
   if mem.active then
      po:progress( mem.timer / active )
      if mem.timer < 0 then
         turnoff( p, po )
      end
   else
      po:progress( mem.timer / cooldown )
      if mem.timer < 0 then
         po:state("off")
         mem.timer = nil
      end
   end
end

function ontoggle( p, po, on )
   if on then
      return turnon( p, po )
   end
end
