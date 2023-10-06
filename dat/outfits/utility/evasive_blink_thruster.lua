local audio = require 'love.audio'
local luaspfx = require 'luaspfx'

local masslimit = 800^2 -- squared
local jumpdist = 300
local cooldown = 1

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

local function doblink( p, po, blinkdir )
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
   local pos, vel = p:pos(), p:vel()
   luaspfx.blink( p, pos ) -- Blink afterimage
   p:effectAdd( "Blink" ) -- Cool "blink in" effect
   local dir = p:dir()
   --[[
   local px, py = math.cos(dir), math.sin(dir)
   local vx, vy = vel:get()
   if px*vy - py*vx < 0 then -- z component of cross product
   --]]
   if blinkdir > 0 then
      dir = dir - math.pi*0.5
   else
      dir = dir + math.pi*0.5
   end
   p:setPos( pos + vec2.newP( dist, dir ) )
   mem.timer = cooldown * p:shipstat("cooldown_mod",true)
   po:state("cooldown")
   po:progress(1)

   -- Play the sound
   luaspfx.sfx( pos, vel, sfx )

   return true
end

function ontoggle( p, po, on )
   -- Only care about turning on (outfit never has the "on" state)
   if not on then return false end
   local dir
   if rnd.rnd() > 0.5 then
      dir = 1
   else
      dir = -1
   end
   return doblink( p, po, dir )
end

function keydoubletap( p, po, key )
   if key=="left" then
      doblink( p, po, -1 )
   elseif key=="right" then
      doblink( p, po, 1 )
   end
end
