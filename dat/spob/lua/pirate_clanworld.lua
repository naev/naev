local pir = require "common.pirate"
local luaspob = require "spob.lua.lib.spob"

function load( spb )
   local std_land = 20
   if pir.isPirateShip( player.pilot() ) then
      std_land = 0
   end
   return luaspob.setup( spb, {
      bribe_cost = function ()
         local std = spb:faction():playerStanding()
         return (std_land - std) * 500 + 1000
      end,
      std_land = std_land,
      std_bribe = -50,
      msg_granted = {
         _([["Permission to land granted. Welcome, mate."]]),
      },
      msg_denied = {
         _([["Get out of here!"]]),
      },
      msg_notyet = {
         _([["Small time pirates have no business on our clanworld!"]])
      },
      msg_cantbribe = {
         _([["Money won't buy you access to our restricted facilities, citizen."]]),
      },
      msg_trybribe = {
         _([["Well, I think you're scum, but I'm willing to look the other way for {credits}. Deal?"]]),
      },
      msg_bribed = {
         _([["Heh heh, thanks. Now get off the comm, I'm busy!"]]),
      },
   } )
end

can_land = luaspob.can_land
comm = luaspob.comm
