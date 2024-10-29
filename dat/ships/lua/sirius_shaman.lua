local flow = require "ships.lua.lib.flow"
local fmt = require "format"
require "ships.lua.sirius"

function descextra( _p, _s )
   return "#y"..fmt.f(_("For every {over} flow above {min} flow, increases launcher fire rate and range by {bonus}% and lock-on, calibration, and reload rates by {bonus2}% up to a maximum of {max}% and {max2}, respectively."),
      {over=10, min=40, bonus=5, bonus2=10, max=40, max2=80}).."#0"
end

function update( p, _dt )
   local f = flow.get( p, mem )
   local mod = math.min( 40, math.max( (f-40)*0.5, 0 ) )
   p:shippropSet{
      launch_rate = mod,
      launch_range = mod,
      launch_lockon = -mod*2,
      launch_calibration = -mod*2,
      launch_reload = mod*2,
   }
end
