local flow = require "ships.lua.lib.flow"
local spfx_rip = require "luaspfx.realityrip"

local flow_drain, flow_cost, field_range

function onload( o )
   -- TODO make outfit specific
   if o==outfit.get("Reality Rip") then
      flow_drain  = 8
      flow_cost   = 80
      field_range = 500
   end
end

local function turnon( p, po )
   local f = flow.get( p )
   if f < flow_cost then
      return false
   end
   flow.dec( p, flow_cost )

   po:state("on")
   po:progress( flow.get(p) / flow.max(p) )
   mem.field = spfx_rip( p:pos(), field_range )
end

local function turnoff( _p, po )
   if mem.field then
      mem.field:rm()
      mem.field = nil
   end
   po:state("off")
end

function init( p, po )
   mem.isp = (player.pilot()==p) -- is player?

   turnoff( p, po )
end

function update( p, po, dt )
   if mem.field then
      --  Drain flow
      flow.dec( p, dt * flow_drain )
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
