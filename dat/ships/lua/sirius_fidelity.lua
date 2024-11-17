local flow = require "ships.lua.lib.flow"
local fmt = require "format"
require "ships.lua.sirius"

function descextra( _p, _s )
 return "#y"..fmt.f(_("For each {over} flow over {min} flow, increases action speed by {bonus}% up to a maximum of {max}%."),
   {over=10, min=25, bonus=3, max=25}).."#0"
end

function update( p, _dt )
   local f = flow.get( p, mem )
   -- Maximum at 108.3333 flow
   local mod = math.min( 25, math.max( (f-25)*0.3, 0 ) )
   p:shippropSet{
      time_speedup = mod,
   }
end
