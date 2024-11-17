local flow = require "ships.lua.lib.flow"
local fmt = require "format"
require "ships.lua.sirius"

function descextra( _p, _s )
   return "#y"..fmt.f(_("For each {over} flow above {min} flow, increases shield regeneration rate by {inc}% up to {max}%."),
      {over=10, min=80, inc=4, max=80}).."#0"
end

function update( p, _dt )
   local f = flow.get( p, mem )
   local mod = math.max( (f-80)*0.4, 0 )
   p:shippropSet{
      shield_regen = mod,
   }
end
