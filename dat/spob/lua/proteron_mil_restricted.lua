local luaspob = require "spob.lua.lib.spob"

function init( spb )
   return luaspob.init( spb, {
      std_land = 50,
      std_bribe = 100,
      msg_granted = {
         _("Landing access authorized."),
      },
      msg_notyet = {
         _([["You are not authorized to land here."]]),
      },
      msg_cantbribe = {
         _([["The Sovereign Proteron Autarchy does not take bribes."]]),
      },
   } )
end

load = luaspob.load
unload = luaspob.unload
can_land = luaspob.can_land
comm = luaspob.comm
