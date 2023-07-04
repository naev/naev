local flow = require "ships.lua.lib.flow"
require "ships.lua.sirius"

function update( p, _dt )
   local f = flow.get( p, mem )
   local mod = math.max( (f-150)*0.4, 0 )
   p:shippropSet{
      absorb = mod,
   }
end
