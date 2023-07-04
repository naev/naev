local flow = require "ships.lua.lib.flow"
require "ships.lua.sirius"

function update( p, _dt )
   local f = flow.get( p, mem )
   local mod = math.max( (f-40)*0.5, 0 )
   p:shippropSet{
      launch_rate = mod,
      launch_range = mod,
      launch_lockon = -mod*2,
      launch_calibration = -mod*2,
      launch_reload = mod*2,
   }
end
