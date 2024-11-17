local flow = require "ships.lua.lib.flow"
local fmt = require "format"
require "ships.lua.sirius"

function descextra( _p, _s )
   return "#y"..fmt.f(_("For every {over} flow over {min}, increases damage absorption by {inc}% up to {max}%."),
      {over=100, min=150, inc=15, max=60}).."#0"
end

function update( p, _dt )
   local f = flow.get( p, mem )
   local mod = math.min( 60, math.max( (f-150)*0.4, 0 ) )
   p:shippropSet{
      absorb = mod,
   }
end
