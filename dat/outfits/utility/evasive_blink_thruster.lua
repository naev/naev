local audio = require 'love.audio'
local luaspfx = require 'luaspfx'

local masslimit = 500^2 -- squared
local jumpdist = 300
local cooldown = 3

local sfx = audio.newSource( 'snd/sounds/blink.ogg' )

function init( p, po )
   po:state("off")
   mem.timer = 0
   mem.isp = (p == player.pilot())
   po:set("ew_signature",-10) -- Have to set here because the outfit is never "on"

   if mem.isp then
      for k,v in ipairs(p:outfits()) do
         if v and v:typeBroad()=="Afterburner" then
            mem.afterburner = true
            break
         end
      end
   end
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
   p:setPos( pos + vec2.newP( dist, p:dir()+blinkdir ) )
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
   return doblink( p, po, 0 ) -- Goes forward
end

function keydoubletap( p, po, key )
   -- Only blink forward on double tap if no afterburner
   if not mem.afterburner and key=="accel" then
      doblink( p, po, 0 )
   elseif key=="left" then
      doblink( p, po, math.pi*0.5 )
   elseif key=="right" then
      doblink( p, po, -math.pi*0.5 )
   end
end
