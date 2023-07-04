local flow = require "ships.lua.lib.flow"
require "ships.lua.sirius"

function update( p, _dt )
   local f = flow.get( p, mem )
   local mod = math.max( (f-25)*0.4, 0 )
   p:shippropSet{
      time_speedup = mod,
   }
end
