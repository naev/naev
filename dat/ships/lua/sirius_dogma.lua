local flow = require "ships.lua.lib.flow"
local fmt = require "format"
require "ships.lua.sirius"

function descextra( _p, _s )
   return "#y"..fmt.f(_("Gains {inc}% weapon damage per {over} flow over {min} flow up to {max}%."),
      {inc=4, over=100, min=250, max=40}).."#0"
end

function update( p, _dt )
   local f = flow.get( p, mem )
   local mod = math.min( 40, math.max( (f-250)*0.04, 0 ) )
   p:shippropSet{
      fwd_damage = mod,
      tur_damage = mod,
      launch_damage = mod,
   }
end
