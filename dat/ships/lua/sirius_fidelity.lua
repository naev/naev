local flow = require "ships.lua.lib.flow"
require "ships.lua.sirius"

function update( p, _dt )
   local f = flow.get( p, mem )
   -- Maximum at 108.3333 flow
   local mod = math.min( 25, math.max( (f-25)*0.3, 0 ) )
   p:shippropSet{
      time_speedup = mod,
   }
end
