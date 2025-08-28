local audio = require 'love.audio'
local luaspfx = require 'luaspfx'
local fmt = require "format"
local helper = require "outfits.lib.helper"
local blib = require "outfits.lib.blink"

local limit = 6000
local masslimit = limit^2 -- squared
local jumpdist = 2000
local cooldown = 8
local warmup = 2
local penalty = -50
local energy = 400

local sfx = audio.newSource( 'snd/sounds/blink.ogg' )
local sfx_warmup = audio.newSource( 'snd/sounds/activate1.ogg' )

function descextra( _p, _o )
   return fmt.f(_([[Warms up for {warmup} seconds, then blinks you {jumpdist} {unit_dist} roughly in the direction you are facing. Can only be run once every {cooldown} seconds and incurs a {penalty}% accel and turn penalty during cooldown. Performance degrades over {masslimit} of mass. Blinking costs {energy} {unit_energy} energy.]]), {
      warmup = warmup,
      jumpdist = fmt.number(jumpdist),
      cooldown = cooldown,
      masslimit = fmt.tonnes(limit),
      energy = tostring(energy),
      unit_dist = naev.unit("distance"),
      unit_energy = naev.unit("energy"),
      penalty = penalty,
   })
end

function init( p, po )
   po:state("off")
   mem.timer = 0
   mem.warmup = false
   mem.isp = (p == player.pilot())
end

function update( p, po, dt )
   mem.timer = mem.timer - dt
   if mem.timer < 0 then
      if mem.warmup then
         -- Blink!
         mem.warmup = false
         local dist = jumpdist * blib.bonus_range( p )
         local m = p:mass()
         m = m*m
         -- We use squared values here so twice the masslimit is 25% efficiency
         if m > masslimit then
            dist = dist * masslimit / m
         end
         local pos = p:pos()
         luaspfx.blink( p, pos ) -- Blink afterimage
         p:effectAdd( "Blink" ) -- Cool "blink in" effect
         -- Direction is random
         p:setPos( pos + vec2.newP( dist, p:dir()+(2*rnd.rnd()-1)*math.pi/6 ) )

         -- Play the sound
         if mem.isp then
            luaspfx.sfx( true, nil, sfx )
         else
            luaspfx.sfx( pos, p:vel(), sfx )
         end

         -- Set cooldown and maluses
         po:state("cooldown")
         po:progress(1)
         po:set("accel_mod", penalty)
         po:set("turn_mod", penalty)
         mem.cooldown = true
         mem.timer = cooldown * p:shipstat("cooldown_mod",true)
      else
         -- Cooldown is over
         if mem.cooldown then
            po:state("off")
            -- Cancel maluses
            po:clear()
            mem.cooldown = false
         end
      end
   else
      -- Update progress
      if mem.warmup then
         po:progress( mem.timer / warmup )
      else
         po:progress( mem.timer / cooldown )
      end
   end
end

local function doblink( p, po )
   -- Not ready yet
   if mem.timer > 0 then return false end

   -- Test energy
   if p:energy(true) < energy then
      if mem.isp then
         helper.msgnospam("#r"..fmt.f(_("Not enough energy to use {outfit}!"),{outfit=po:outfit()}).."#0")
      end
      return false
   end

   -- Pay the cost
   p:addEnergy( -energy )

   -- Start the warmup
   mem.warmup = true
   mem.timer = warmup
   po:state("on")
   po:progress(1)
   if mem.isp then
      luaspfx.sfx( true, nil, sfx_warmup )
   else
      luaspfx.sfx( p:pos(), p:vel(), sfx_warmup )
   end
   return true
end

function ontoggle( p, po, on )
   -- Only care about turning on (outfit never has the "on" state)
   if not on then
      mem.lastmsg = nil -- clear helper.msgnospam timer
      return false
   end
   return doblink( p, po )
end
