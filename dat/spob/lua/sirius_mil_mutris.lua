local luaspob = require "spob.lua.lib.spob"

function load( spb )
   return luaspob.setup( spb, {
      std_land = 90,
      std_bribe = 100,
      msg_granted = {
         _([["Welcome to Mutris, home of Sirichana."]]),
      },
      msg_notyet = {
         _([["You may not approach the home of Sirichana yet."]]),
      },
      msg_cantbribe = {
         _([["The faithful will never be swayed by money."]]),
      },
   } )
end

can_land = luaspob.can_land
comm = luaspob.comm
