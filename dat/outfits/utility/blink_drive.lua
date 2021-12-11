local audio = require 'love.audio'

local masslimit = 800^2 -- squared
local jumpdist = 500
local cooldown = 8

local sfx = audio.newSource( 'snd/sounds/blink.ogg' )

function init( p, po )
   po:state("off")
   mem.timer = 0
   mem.isp = (p == player.pilot())
end

function update( _p, po, dt )
   mem.timer = mem.timer - dt
   po:progress( mem.timer / cooldown )
   if mem.timer < 0 then
      po:state("off")
   end
end

function ontoggle( p, po, on )
   -- Only care about turning on (outfit never has the "on" state)
   if not on then return false end

   -- Not ready yet
   if mem.timer > 0 then return false end

   -- Blink!
   local dist = jumpdist
   local m = p:mass()
   m = m*m
   -- We use squared values here so twice the masslimit is 25% efficiency
   if m > masslimit then
      dist = dist * masslimit / m
   end
   p:setPos( p:pos() + vec2.newP( dist, p:dir() ) )
   mem.timer = cooldown
   po:state("cooldown")
   po:progress(1)

   -- TODO play for other pilots
   if mem.isp then
      -- TODO Add blink effect
      sfx:setPitch( player.dt_mod() )
      sfx:play()
   end
   return true
end
