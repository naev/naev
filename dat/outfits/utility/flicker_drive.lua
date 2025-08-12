local audio = require 'love.audio'
local luaspfx = require 'luaspfx'
local fmt = require "format"
local helper = require "outfits.lib.helper"
local blib = require "outfits.lib.blink"

local limit = 500
local masslimit = limit^2 -- squared
local jumpdist = 300
local cooldown = 3
local energy = 40
local ewbonus = -25

local sfx = audio.newSource( 'snd/sounds/blink.ogg' )

function descextra( _p, _o )
   return fmt.f(_([[Blinks you {jumpdist} {unit_dist} forward. Double-tapping left or right lets you blink to the sides. Can only be run once every {cooldown} seconds. Performance degrades over {masslimit} of mass. Blinking costs {energy} {unit_energy} energy.
Gives a #g{ewbonus}% Ship Detectability#0 bonus during cooldown.]]), {
      jumpdist = jumpdist,
      cooldown = cooldown,
      masslimit = fmt.tonnes(limit),
      energy = tostring(energy),
      ewbonus = ewbonus,
      unit_dist = naev.unit("distance"),
      unit_energy = naev.unit("energy"),
   })
end

function init( p, po )
   po:state("off")
   mem.timer = 0
   mem.isp = (p == player.pilot())

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
   if mem.timer <= 0 then
      po:state("off")
      po:set( "ew_hide", 0 )
      po:set( "ew_stealth_min", 0 )
   else
      po:set( "ew_hide", ewbonus )
      po:set( "ew_stealth_min", ewbonus )
   end
end

local function doblink( p, po, blinkdir )
   -- Not ready yet
   if mem.timer > 0 then return false end

   -- Test energy
   if p:energy(true) < energy then
      helper.msgnospam("#r"..fmt.f(_("Not enough energy to use {outfit}!"),{outfit=po:outfit()}).."#0")
      return false
   end

   -- Pay the cost
   p:addEnergy( -energy )

   -- Blink!
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
   p:setPos( pos + vec2.newP( dist, p:dir()+blinkdir ) )
   mem.timer = cooldown * p:shipstat("cooldown_mod",true)
   po:state("cooldown")
   po:progress(1)

   -- Play the sound
   luaspfx.sfx( pos, p:vel(), sfx )

   return true
end

function ontoggle( p, po, on )
   -- Only care about turning on (outfit never has the "on" state)
   if not on then
      mem.lastmsg = nil -- clear helper.msgnospam timer
      return false
   end
   return doblink( p, po, 0 ) -- Goes forward
end

function keydoubletap( p, po, key )
   -- Only blink forward on double tap if no afterburner
   if not mem.afterburner and key=="accel" then
      return doblink( p, po, 0 )
   elseif key=="left" then
      return doblink( p, po, math.pi*0.5 )
   elseif key=="right" then
      return doblink( p, po, -math.pi*0.5 )
   end
end
