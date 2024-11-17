local flow = require "ships.lua.lib.flow"
local spfx_rip = require "luaspfx.realityrip"
local fmt = require "format"

local function getStats( p, size )
   local flow_cost, flow_drain, range, strength
   size = size or flow.size( p )
   if size == 1 then
      flow_cost   = 40
      flow_drain  = 4
      range       = 300
      strength    = 0.75
   elseif size == 2 then
      flow_cost   = 80
      flow_drain  = 8
      range       = 400
      strength    = 1
   else
      flow_cost   = 160
      flow_drain  = 16
      range       = 500
      strength    = 1.25
   end
   return flow_cost, flow_drain, range, strength
end

function descextra( p, _o )
   -- Generic description
   local size
   if p then
      size = flow.size( p )
   else
      size = 0
   end
   local s = "#y".._([[Creates a temporal distortion field that heavily accelerates ships with respect to those outside.]]).."#0"
   for i=1,3 do
      local cost, drain, range, strength = getStats( nil, i )
      local pfx = flow.prefix(i)
      if i==size then
         pfx = "#b"..pfx.."#n"
      end
      s = s.."\n"..fmt.f(_("#n{prefix}:#0 {cost} flow, {drain} flow drain, {range} field range, {strength}% effect strength"),
         {prefix=pfx, cost=cost, drain=drain, range=range, strength=strength*100}).."#0"
   end
   return s
end

local function turnon( p, po )
   local f = flow.get( p )
   if f < mem.flow_cost then
      return false
   end
   flow.dec( p, mem.flow_cost )

   po:state("on")
   po:progress( flow.get(p) / flow.max(p) )
   mem.field = spfx_rip( p:pos(), mem.field_range, {strength=mem.strength} )
end

local function turnoff( _p, po )
   if mem.field then
      mem.field:rm()
      mem.field = nil
   end
   po:state("off")
end

function init( p, po )
   mem.flow_cost, mem.flow_drain, mem.field_range, mem.strength = getStats( p )
   mem.isp = (player.pilot()==p) -- is player?
   turnoff( p, po )
end

function update( p, po, dt )
   if mem.field then
      --  Drain flow
      flow.dec( p, dt * mem.flow_drain )
      if flow.get( p ) <= 0 then
         turnoff( p, po )
         return
      end
      po:progress( flow.get(p) / flow.max(p) )
   end
end

function ontoggle( p, po, on )
   if on then
      -- Not ready yet
      if mem.field then return false end

      return turnon( p, po )
   else
      if mem.field then
         turnoff( p, po )
         return true
      end
   end
   return false
end
