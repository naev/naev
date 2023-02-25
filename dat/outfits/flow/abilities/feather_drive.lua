local audio = require 'love.audio'
local luaspfx = require 'luaspfx'
local flow = require "ships.lua.lib.flow"

local flow_cost, range, masslimit
local cooldown

local sfx = audio.newSource( 'snd/sounds/blink.ogg' )

function onload( o )
   if o==outfit.get("Lesser Feather Drive") then
      flow_cost   = 40
      range       = 500
      masslimit   = 500^2 --squared
      cooldown    = 7
   else
      error(_("Unknown outfit using script!"))
   end
end

function init( _p, po )
   mem.timer = 0
   po:state("off")
end

function update( _p, po, dt )
   if mem.timer < 0 then return end

   mem.timer = mem.timer - dt
   if mem.timer < 0 then
      po:state("off")
   else
      po:progress( mem.timer / cooldown )
   end
end

function ontoggle( p, po, on )
   -- Only care about turning on (outfit never has the "on" state)
   if not on then return false end

   -- Not ready yet
   if mem.timer > 0 then return false end

   -- Use flow
   local f = flow.get( p )
   if f < flow_cost then
      return false
   end
   flow.dec( p, flow_cost )

   -- Blink!
   local dist = range
   local m = p:mass()
   m = m*m
   -- We use squared values here so twice the masslimit is 25% efficiency
   if m > masslimit then
      dist = dist * masslimit / m
   end
   local pos = p:pos()
   luaspfx.blink( pos ) -- Blink afterimage
   p:effectAdd( "Blink" ) -- Cool "blink in" effect
   p:setPos( pos + vec2.newP( dist, p:dir() ) )
   mem.timer = cooldown * p:shipstat("cooldown_mod",true)
   po:state("cooldown")
   po:progress(1)

   -- Play the sound
   luaspfx.sfx( pos, p:vel(), sfx )

   return true
end
