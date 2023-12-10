local audio = require 'love.audio'
local luaspfx = require 'luaspfx'
local flow = require "ships.lua.lib.flow"
local fmt = require "format"

local sfx = audio.newSource( 'snd/sounds/activate4.ogg' )

local function getStats( p, size )
   local flow_cost, flow_drain, mass_limit
   size = size or flow.size( p )
   if size == 1 then
      flow_drain  = 8
      flow_cost   = 20
      mass_limit  = 500
   elseif size == 2 then
      flow_drain  = 16
      flow_cost   = 40
      mass_limit  = 3000
   else
      flow_drain  = 32
      flow_cost   = 80
      mass_limit  = 15e3
   end
   return flow_cost, flow_drain, mass_limit
end

function descextra( p, _o )
   -- Generic description
   local size
   if p then
      size = flow.size( p )
   else
      size = 0
   end
   local s = "#y"..fmt.f(_([[Uses flow to enhance the ships damage by {dmg}%, movement by {mvt}%, absorption by {abs}%, jamming chance by {jam}%, and shield and energy regeneration by {regen}. Furthermore, it accelerates the ship by {accel}%. ]]),
      {dmg=25, mvt=25, abs=20, jam=30, regen=50, accel=25}).."#0"
   for i=1,3 do
      local cost, drain, limit = getStats( nil, i )
      local pfx = flow.prefix(i)
      if i==size then
         pfx = "#b"..pfx.."#n"
      end
      s = s.."\n"..fmt.f(_("#n{prefix}:#0 {cost} flow, drains {drain} flow per second, limited to ships under {limit}"),
         {prefix=pfx, cost=cost, drain=drain, limit=fmt.tonnes_short(limit)}).."#0"
   end
   return s
end

local function turnon( p, po )
   if p:mass() > mem.mass_limit then
      player.msg("#r".._("Your ship is above the mass limit to use Avatar of Sirichana.").."#0")
      return false
   end

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
   mem.flow_cost, mem.flow_drain, mem.mass_limit = getStats( p )

   po:state("off")
   mem.isp = (p == player.pilot())
   turnoff( p, po )
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
