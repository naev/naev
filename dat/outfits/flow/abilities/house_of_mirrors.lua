local aisetup = require "ai.core.setup"
local flow = require "ships.lua.lib.flow"
local fmt = require "format"

local function getStats( p, size )
   local flow_cost, flow_drain, copies
   size = size or flow.size( p )
   if size == 1 then
      flow_drain  = 8
      flow_cost   = 30
      copies      = 2
   elseif size == 2 then
      flow_drain  = 16
      flow_cost   = 60
      copies      = 3
   else
      flow_drain  = 32
      flow_cost   = 120
      copies      = 4
   end
   return flow_cost, flow_drain, copies
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
      local cost, drain, copies = getStats( nil, i )
      local pfx = flow.prefix(i)
      if i==size then
         pfx = "#b"..pfx.."#n"
      end
      s = s.."\n"..fmt.f(_("#n{prefix}:#0 {cost} flow, {drain} flow drain per second, {copies} copies"),
         {prefix=pfx, cost=cost, drain=drain, copies=copies}).."#0"
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

   local pw, ph = p:ship():dims()
   mem.r    = (pw+ph)*0.5
   mem.off  = p:dir()

   -- Create figures
   mem.p = {}
   for i=1,mem.n do
      local pos = vec2.newP( mem.r, mem.off + math.pi*2*i/mem.n )
      local np = pilot.add( _("Mirror"), p:faction(), pos, p:name(), {ai="house_of_mirrors", naked=true} )
      np:effectAdd("Astral Projection")
      np:outfitRm("all")
      for k,v in ipairs(p:outfitsList()) do
         np:outfitAdd( v, 1, true )
      end
      aisetup.setup( np ) -- Initialize AI
      np:intrinsicSet( {
         launch_damage  = -80,
         fbay_damage    = -80,
         fwd_damage     = -80,
         tur_damage     = -80,
      }, true ) -- overwrite all
      np:setDir( p:dir() )
      np:setVel( p:vel() )
      np:control( true )
      np:setLeader( p )
      np:setInvisible( true )
      mem.p[i] = np
   end

   flow.activate( p )
   return true
end

local function removeprojection( p )
   if mem.p then
      -- Remove all potential escorts
      for k,np in ipairs(mem.p) do
         np:rm()
      end
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
      --  Drain flow
      flow.dec( p, dt * mem.flow_drain )
      if flow.get( p ) <= 0 then
         turnoff( p, po )
         return
      end
      po:progress( flow.get(p) / flow.max(p) )

      -- Spin them around and shoot
      mem.off = mem.off + 0.2 * math.pi * dt
      local t = p:target()
      for k,np in ipairs(mem.p) do
         -- Update position
         --np:setDir( p:dir() )
         np:setVel( p:vel() )
         local pos = vec2.newP( mem.r, mem.off + math.pi*2*k/mem.n )
         np:setPos( pos )

         -- Shoot
         np:taskClear()
         if t then
            np:pushtask( "shootat", t )
         end
      end
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
