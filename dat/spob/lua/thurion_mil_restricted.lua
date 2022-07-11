local fmt = require "format"
local luaspob = require "spob.lua.lib.spob"

function load( spb )
   return luaspob.setup( spb, {
      std_land = 50,
      std_bribe = 100,
      msg_granted = {
         fmt.f(_([["Welcome, friend {player}. You may dock when ready."]]), {player=player.name()}),
      },
      msg_notyet = {
         _([["We can't trust you to land here just yet."]]),
      },
      msg_cantbribe = {
         _([["We have no need for your credits."]]),
      },
   } )
end

can_land = luaspob.can_land
comm = luaspob.comm
