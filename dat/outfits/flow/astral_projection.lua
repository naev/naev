local flow = require "ships.lua.lib.flow"

local flow_drain, flow_cost, projection

function onload( _o )
   -- TODO make outfit specific
   flow_drain = 1
   flow_cost = 40
   projection = "Llama"
end

local function turnon( p, po )
   local f = flow.get( p )
   if f < flow_cost then
      return false
   end
   flow.dec( p, flow_cost )

   -- Set outfit state
   po:state("on")
   po:progress(1)

   -- Astral proection
   local s = p:ship()
   local pos = p:pos() + vec2.newP( 20, s:dir() )
   local np = pilot.add( projection, p:faction(), pos, _("Lesser Astral Projection"), {ai="escort"} )
   mem.p = np
   np:setNoDeath( true ) -- Dosen't die

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
      flow.dec( p, dt * flow_drain )
      if flow.get( p ) <= 0 then
         turnoff( p, po )
         return
      end
   end
end

-- This should trigger when the pilot is disabled or killed and destroy the
-- hologram if it is up
function ontoggle( p, po, on )
   if on then
      -- Not ready yet
      if mem.p then return false end

      return turnon( p, po )
   else
      if mem.p then
         turnoff( po )
         return true
      end
   end
   return false
end
