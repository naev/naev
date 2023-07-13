notactive = true -- Doesnt' become active

local flow = require "ships.lua.lib.flow"
local fmt = require "format"
local srs = require "common.sirius"

function descextra( _p, o )
   local powers = ""
   if not srs.playerIsPsychic() then
      powers = "\n#r".._("Requires psychic powers to use.").."#0"
   end
   return fmt.f(_("Provides {flow} maximum flow capacity and allows a ship to use flow and allows gaining 10% of damage received as flow.{powers}"),
      { flow=flow.list_base[o:nameRaw()], powers=powers })
end

function init( p )
   flow.recalculate( p )
end

function update( p, _po, dt )
   flow.update( p, dt )
end

function onhit( p, _po, armour, shield )
   flow.onhit( p, armour, shield )
end
