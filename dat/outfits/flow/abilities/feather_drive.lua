local audio = require 'love.audio'
local luaspfx = require 'luaspfx'
local flow = require "ships.lua.lib.flow"
local fmt = require "format"

local sfx = audio.newSource( 'snd/sounds/blink.ogg' )

function onadd( _p, po )
   local size = po:slot().size
   if size=="Small" then
      mem.flow_cost   = 40
      mem.range       = 500
      mem.masslimit   = 500^2 --squared
      mem.cooldown    = 7
   else
      error(fmt.f(_("Flow ability '{outfit}' put into slot of unknown size '{size}'!"),
         {outfit=po:outfit(),size=size}))
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
      po:progress( mem.timer / mem.cooldown )
   end
end

function ontoggle( p, po, on )
   -- Only care about turning on (outfit never has the "on" state)
   if not on then return false end

   -- Not ready yet
   if mem.timer > 0 then return false end

   -- Use flow
   local f = flow.get( p )
   if f < mem.flow_cost then
      return false
   end
   flow.dec( p, mem.flow_cost )

   -- Blink!
   local dist = mem.range
   local m = p:mass()
   m = m*m
   -- We use squared values here so twice the masslimit is 25% efficiency
   if m > mem.masslimit then
      dist = dist * mem.masslimit / m
   end
   local pos = p:pos()
   luaspfx.blink( pos ) -- Blink afterimage
   p:effectAdd( "Blink" ) -- Cool "blink in" effect
   p:setPos( pos + vec2.newP( dist, p:dir() ) )
   mem.timer = mem.cooldown * p:shipstat("cooldown_mod",true)
   po:state("cooldown")
   po:progress(1)

   -- Play the sound
   luaspfx.sfx( pos, p:vel(), sfx )

   return true
end
