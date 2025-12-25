local flow = require "ships.lua.lib.flow"
local fmt = require "format"
require "ships.lua.sirius"

local MAXBONUS = 25  -- Maximum value of the bonus
local MINFLOW  = 25  -- Flow required to start getting a bonus
local OVERFLOW = 10  -- How much bonus is needed for an increment
local INCBONUS = 3   -- Increment per OVERFLOW
local BONUS    = INCBONUS / OVERFLOW

function descextra( _p, _s )
   return "#y"..fmt.f(_("For each {over} flow over {min} flow, increases action speed by {bonus}% up to a maximum of {max}%."),
      {over=OVERFLOW, min=MINFLOW, bonus=INCBONUS, max=MAXBONUS}).."#0"
end

function update( p, _dt )
   local f = flow.get( p, mem )
   -- Maximum at 108.3333 flow
   local mod = math.min( MAXBONUS, math.max( (f-MINFLOW)*BONUS, 0 ) )
   p:shippropSet{
      action_speed = mod,
   }
end
