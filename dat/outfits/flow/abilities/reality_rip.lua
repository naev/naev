local flow = require "ships.lua.lib.flow"
local spfx_rip = require "luaspfx.realityrip"
local fmt = require "format"

function onadd( _p, po )
   local size = po:slot().size
   -- Doesn't have small version
   if size=="Medium" then
      mem.flow_drain  = 8
      mem.flow_cost   = 80
      mem.field_range = 500
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

   po:state("on")
   po:progress( flow.get(p) / flow.max(p) )
   mem.field = spfx_rip( p:pos(), mem.field_range )
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
