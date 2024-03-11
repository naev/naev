notactive = true -- Doesnt' become active

local flow = require "ships.lua.lib.flow"
local fmt = require "format"
local srs = require "common.sirius"

function descextra( p, o )
   local powers = ""
   if not srs.playerIsPsychic() then
      powers = "\n#r".._("Requires psychic powers to use.").."#0"
   end
   local oname = o:nameRaw()
   local amount
   if oname=="Internal Flow Amplifier" then
      if p then
         amount = flow.list_base[p:ship():nameRaw()]
      else
         amount = _("??")
      end
   else
      amount = flow.list_base[o:nameRaw()]
   end
   return fmt.f("#y".._("Provides {flow} maximum flow capacity and allows a ship to use flow and allows gaining 10% of damage received as flow.{powers}").."#0",
      { flow=amount, powers=powers })
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
