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
         _([["Scram before I blast you to pieces myself."]]),
         _([["Go away!"]]),
      },
      msg_notyet = {
         _([["Small time pirates have no business on our clanworld!"]]),
         _([["Come back when you're a real pirate."]]),
         _([["Didn't you read the 'Real Pirates Only' sign? Go away."]])
      },
      msg_cantbribe = {
         _([["Your money isn't worth the trouble you bring."]]),
      },
      msg_trybribe = {
         _([["Well, I think you're scum, but I'm willing to look the other way for {credits}. Deal?"

Pay {credits}?]]),
         _([["I could use some more wax for my Pirate Hyena. I'll make a landing exception for your ship for {credits}."

Pay {credits}?]]),
      },
      msg_bribed = {
         _([["Heh heh, thanks. Now get off the comm, I'm busy!"]]),
         _([["Please doing business with you. Now get moving."]]),
      },
   } )
end

can_land = luaspob.can_land
comm = luaspob.comm
