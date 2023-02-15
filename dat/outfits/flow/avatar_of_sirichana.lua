local audio = require 'love.audio'
local luaspfx = require 'luaspfx'
local flow = require "ships.lua.lib.flow"

local flow_drain, flow_cost

local sfx = audio.newSource( 'snd/sounds/activate4.ogg' )

function onload( _o )
   -- TODO make outfit specific
   flow_drain = 8
   flow_cost = 40
end

local function turnon( p, po )
   local f = flow.get( p )
   if f < flow_cost then
      return false
   end
   flow.dec( p, flow_cost )

   -- Turn on
   po:state("on")
   po:progress( flow.get(p) / flow.max(p) )
   mem.active = true

   -- TODO add effect to pilot

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
      flow.dec( p, dt * flow_drain )
      if flow.get( p ) <= 0 then
         turnoff( p, po )
         return
      end
      po:progress( flow.get(p) / flow.max(p) )

      -- TODO update effect on pilot
   end
end

function ontoggle( p, po, on )
   if on then
      return turnon( p, po )
   else
      return turnoff( p, po )
   end
end
