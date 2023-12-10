local audio = require 'love.audio'
local luaspfx = require 'luaspfx'
local fmt = require "format"
local helper = require "outfits.lib.helper"

local masslimit = 1000^2 -- squared
local jumpdist = 500
local cooldown = 7
local energy = 140
local heat

local sfx = audio.newSource( 'snd/sounds/blink.ogg' )

function onload( o )
   heat = o:heatFor( 30/cooldown ) -- Roughly overheat in 30 s of continuous use (much more in reality)
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
   if mem.timer < 0 then
      po:state("off")
   end
end

local function doblink( p, po, blinkdir, strength )
   -- Not ready yet
   if mem.timer > 0 then return false end

   -- Test energy
   if p:energy(true) < energy then
      helper.msgnospam("#r"..fmt.f(_("Not enough energy to use {outfit}!"),{outfit=po:outfit()}).."#0")
      return false
   end

   -- Test heat
   local h = po:heat()
   if h <= 0 then
      helper.msgnospam("#r"..fmt.f(_("{outfit} is overheating!"),{outfit=po:outfit()}).."#0")
      return false
   end

   -- Pay the cost
   p:addEnergy( -energy )
   po:heatup( heat )

   -- Blink!
   local dist = jumpdist
   local m = p:mass()
   m = m*m
   -- We use squared values here so twice the masslimit is 25% efficiency
   if m > masslimit then
      dist = dist * masslimit / m
   end
   local pos = p:pos()
   luaspfx.blink( p, pos ) -- Blink afterimage
   p:effectAdd( "Blink" ) -- Cool "blink in" effect
   p:setPos( pos + vec2.newP( h*dist*strength, p:dir()+blinkdir ) )
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
   return doblink( p, po, 0, 1 ) -- Goes forward
end

function keydoubletap( p, po, key )
   -- Only blink forward on double tap if no afterburner
   if not mem.afterburner and key=="accel" then
      doblink( p, po, 0, 1 )
   elseif key=="left" then
      doblink( p, po, math.pi*0.5, 0.5 )
   elseif key=="right" then
      doblink( p, po, -math.pi*0.5, 0.5 )
   end
end
