local audio = require 'love.audio'
local luaspfx = require 'luaspfx'
local flow = require "ships.lua.lib.flow"
local fmt = require "format"

local sfx = audio.newSource( 'snd/sounds/activate4.ogg' )

function onadd( _p, po )
   local size = po:slot().size
   if size=="Small" then
      mem.flow_drain  = 8
      mem.flow_cost   = 40
   elseif size=="Medium" then
      mem.flow_drain  = 16
      mem.flow_cost   = 80
   elseif size=="Large" then
      mem.flow_drain  = 32
      mem.flow_cost   = 160
   else
      error(fmt.f(_("Flow ability '{outfit}' put into slot of unknown size '{size}'!"),
         {outfit=po:outfit(),size=size}))
   end
end

local function turnon( p, po )
   local f = flow.get( p )
   if f < mem.flow_cost then
      return false
   end
   flow.dec( p, mem.flow_cost )

   -- Turn on
   po:state("on")
   po:progress( flow.get(p) / flow.max(p) )
   mem.active = true

   -- Apply effect
   p:effectAdd("Avatar of Sirichana")

   -- Sound effect
   if mem.isp then
      luaspfx.sfx( true, nil, sfx )
   else
      luaspfx.sfx( p:pos(), p:vel(), sfx )
   end

   flow.activate( p )
   return true
end

local function turnoff( p, po )
   if not mem.active then
      return false
   end
   po:state("off")
   mem.active = false
   flow.deactivate( p )
   return true
end

function init( p, po )
   po:state("off")
   mem.isp = (p == player.pilot())
   turnoff()
end

function update( p, po, dt )
   if mem.active then
      --  Drain flow
      flow.dec( p, dt * mem.flow_drain )
      if flow.get( p ) <= 0 then
         turnoff( p, po )
         return
      end
      po:progress( flow.get(p) / flow.max(p) )

      -- Reapply effect
      p:effectAdd("Avatar of Sirichana")
   end
end

function ontoggle( p, po, on )
   if on then
      return turnon( p, po )
   else
      return turnoff( p, po )
   end
end
