local audio = require 'love.audio'
local luaspfx = require 'luaspfx'

local active = 8 -- active time in seconds
local cooldown = 30 -- cooldown time in seconds

local sfx = audio.newSource( 'snd/sounds/activate4.ogg' )

local function turnon( p, po )
   -- Still on cooldown
   if mem.timer and mem.timer > 0 then
      return false
   end
   -- Try to stealth and only activate on stealth
   po:state("on")
   p:calcStats() -- Have to try to stealth with new stats
   if not p:tryStealth() then
      po:state("off")
      return false
   end
   po:progress(1)
   mem.timer = active
   mem.active = true

   -- Visual effect
   if mem.isp then
      luaspfx.sfx( true, nil, sfx )
   else
      luaspfx.sfx( p:pos(), p:vel(), sfx )
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
   turnoff()
   mem.timer = nil
   po:state("off")
   po:clear() -- clear stat modifications
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
