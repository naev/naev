local pir = require "common.pirate"
local luaspob = require "spob.lua.lib.spob"

mem.params = {
   bribe_cost = function ()
      local std = mem.spob:faction():playerStanding()
      return (mem.std_land - std) * 500 + 1000
   end,
   std_land = 20,
   std_bribe = -70,
   std_dangerous = -math.huge,
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
      _([["You think we'd let scum like you land? You're a menace to pirate society!"]]),
      _([["Even pirates have standards better than letting vermin like you land."]]),
   },
   msg_trybribe = {
      _([["Well, I think you're scum, but I'm willing to look the other way for {credits}. Deal?"

Pay {credits}?]]),
      _([["I could use some more wax for my Pirate Hyena. I'll make a landing exception for your ship for {credits}."

Pay {credits}?]]),
   },
   msg_bribed = {
      _([["Heh heh, thanks. Now get off the comm, I'm busy!"]]),
      _([["Pleasure doing business with you. Now get moving."]]),
   },
}

local function initparams ()
   mem.std_land = 20
   local pp = player.pilot()
   if pp:exists() and  pir.isPirateShip( pp ) then
      mem.std_land = 0
   end
   mem.params.std_land = mem.std_land
   return luaspob.init( mem.spob, mem.params )
end

function init( spb )
   mem.spob = spb
   return initparams()
end

load = function ()
   initparams()
   return luaspob.load()
end
unload = luaspob.unload
can_land = luaspob.can_land
comm = luaspob.comm
