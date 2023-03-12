local flow = require "ships.lua.lib.flow"
local fmt = require "format"

local function getStats( p, size )
   local flow_cost, flow_drain, projection
   size = size or flow.size( p )
   if size == 1 then
      flow_drain = 1
      flow_cost = 40
      projection = ship.get("Astral Projection Lesser")
   elseif size == 2 then
      flow_drain = 2
      flow_cost = 80
      projection = ship.get("Astral Projection")
   else
      flow_drain = 4
      flow_cost = 160
      projection = ship.get("Astral Projection Greater")
   end
   return flow_cost, flow_drain, projection
end

function descextra( p, _o )
   -- Generic description
   local size
   if p then
      size = flow.size( p )
   else
      size = 0
   end
   local s = "#y".._([[TODO]]).."#0"
   for i=1,3 do
      local cost, drain = getStats( nil, i )
      local pfx = flow.prefix(i)
      if i==size then
         pfx = "#b"..pfx.."#n"
      end
      s = s.."\n"..fmt.f(_("#n{prefix}:#0 {cost} flow, {drain} flow drain per second"),
         {prefix=pfx, cost=cost, drain=drain}).."#0"
   end
   return s
end

local function turnon( p, po )
   local f = flow.get( p )
   if f < mem.flow_cost then
      return false
   end
   flow.dec( p, mem.flow_cost )

   -- Set outfit state
   po:state("on")
   po:progress( flow.get(p) / flow.max(p) )

   -- Astral proection
   local pos = p:pos() + vec2.newP( 20, p:dir() )
   local np = pilot.add( mem.projection, p:faction(), pos, _("Lesser Astral Projection"), {ai="escort"} )
   mem.p = np
   np:effectAdd("Astral Projection")

   -- Don't let player attack their own astral projection
   if mem.isp then
      np:setInvincPlayer(true)
   end

   -- Exact same position and direction as pilot
   np:setDir( p:dir() )
   np:setVel( p:vel() )
   np:setLeader( p )

   flow.activate( p )
   return true
end

local function removeprojection( p )
   -- get rid of hologram if exists
   if mem.p and mem.p:exists() then
      -- Remove all potential escorts
      for k,f in ipairs(mem.p:followers()) do
         f:rm()
      end
      mem.p:rm()
   end
   if mem.p then
      flow.deactivate( p )
   end
   mem.p = nil
end

local function turnoff( p, po )
   removeprojection( p )
   po:state("off")
end

function init( p, po )
   mem.flow_cost, mem.flow_drain, mem.projection = getStats( p )

   mem.isp = (player.pilot()==p) -- is player?
   turnoff( p, po )
end

function update( p, po, dt )
   if mem.p then
      if not mem.p:exists() then
         turnoff( p, po )
         return
      end

      --  Drain flow
      flow.dec( p, dt * mem.flow_drain )
      if flow.get( p ) <= 0 then
         turnoff( p, po )
         return
      end
      po:progress( flow.get(p) / flow.max(p) )
   end
end

-- This should trigger when the pilot is disabled or killed and destroy the
-- hologram if it is up
function ontoggle( p, po, on )
   if on then
      -- Not ready yet
      if mem.p and mem.p:exists() then return false end

      return turnon( p, po )
   else
      if mem.p then
         turnoff( p, po )
         return true
      end
   end
   return false
end
