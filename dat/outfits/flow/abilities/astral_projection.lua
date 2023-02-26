local flow = require "ships.lua.lib.flow"
local fmt = require "format"

function onadd( _p, po )
   local size = po:slot().size
   if size=="Small" then
      mem.flow_drain = 1
      mem.flow_cost = 40
      mem.projection = ship.get("Astral Projection Lesser")
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
      if mem.p then return false end

      return turnon( p, po )
   else
      if mem.p then
         turnoff( p, po )
         return true
      end
   end
   return false
end
