local flow = require "ships.lua.lib.flow"
local fmt = require "format"

local function getStats( p, size )
   local flow_cost, cooldown, ref, range
   size = size or flow.size( p )
   if size == 1 then
      flow_cost   = 40
      ref         = nil
      range       = 200
      cooldown    = 5
   --elseif size == 2 then
   --else
   end
   return flow_cost, ref, range, cooldown
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
      local cost, _ref, range, cooldown = getStats( nil, i )
      local pfx = flow.prefix(i)
      if i==size then
         pfx = "#b"..pfx.."#n"
      end
      s = s.."\n"..fmt.f(_("#n{prefix}:#0 {cost} flow, {cooldown} s cooldown, {range} range"),
         {prefix=pfx, cost=cost, range=range, cooldown=cooldown}).."#0"
   end
   return s
end

function init( p, po )
   mem.flow_cost, mem.ref, mem.range, mem.cooldown = getStats( p )

   mem.timer = 0
   po:state("off")
end

function update( _p, po, dt )
   if mem.timer < 0 then return end

   mem.timer = mem.timer - dt
   if mem.timer < 0 then
      po:state("off")
   else
      po:progress( mem.timer / mem.cooldown )
   end
end

function ontoggle( p, po, on )
   if on then
      if mem.timer > 0 then
         return
      end

      local f = flow.get( p )
      if f < mem.flow_cost then
         return false
      end
      flow.dec( p, mem.flow_cost )

      -- TODO spfx + damage stuff
      print( mem.ref, mem.range )

      mem.timer = mem.cooldown * p:shipstat("cooldown_mod",true)
      po:state("cooldown")
      po:progress(1)

      return true
   end
   return false
end
