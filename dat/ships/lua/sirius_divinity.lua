local flow = require "ships.lua.lib.flow"
local fmt = require "format"
require "ships.lua.sirius"

update_dt = 1 -- Update once per second
local RANGE = 3000

function descextra( _p, _s )
   return "#y"..fmt.f(_("Every second applies a buff to all fighters within {range} range that increases damage by {inc}% per {over} flow over {min} flow up to {max}%. Range is affected by detection."),
      {range=fmt.number(RANGE), inc=8, over=100, min=250, max=50}).."#0"
end

function update( p )
   local f = flow.get( p, mem )
   local mod = math.min( 50, math.max( (f-250)*0.08, 0 ) )
   if mod > 0 then
      local rmod = p:shipstat("ew_detect",true)
      for k,v in ipairs(p:getAllies( RANGE*rmod )) do
         if v:leader()==p and v:memory().carried then
            v:effectAdd( "Psychic Divinity", nil, mod )
         end
      end
   end
end
