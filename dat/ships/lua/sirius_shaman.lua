local flow = require "ships.lua.lib.flow"
local fmt = require "format"
require "ships.lua.sirius"

local MAXBONUS = 40  -- Maximum value of the bonus
local MINFLOW  = 40 -- Flow required to start getting a bonus
local OVERFLOW = 10  -- How much bonus is needed for an increment
local INCBONUS = 5   -- Increment per OVERFLOW
local BONUS    = INCBONUS / OVERFLOW

function descextra( _p, _s )
   return "#y"..fmt.f(_("For every {over} flow above {min} flow, increases launcher fire rate and range by {bonus}% and lock-on, calibration, and reload rates by {bonus2}% up to a maximum of {max}% and {max2}, respectively."),
      {over=OVERFLOW, min=MINFLOW, bonus=INCBONUS, bonus2=INCBONUS*2, max=MAXBONUS, max2=MAXBONUS*2}).."#0"
end

function update( p, _dt )
   local f = flow.get( p, mem )
   local mod = math.min( MAXBONUS, math.max( (f-MINFLOW)*BONUS, 0 ) )
   p:shippropSet{
      launch_rate = mod,
      launch_range = mod,
      launch_lockon = -mod*2,
      launch_calibration = -mod*2,
      launch_reload = mod*2,
   }
end
