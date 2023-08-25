local luaspob = require "spob.lua.lib.spob"

function init( spb )
   -- Easier to bribe
   return luaspob.init( spb, {
      std_land = -20,
      std_bribe = -50,
      msg_granted = {
         _("Welcome to the Jade Court."),
         _("Landing permission approved."),
      }
   } )
end

load = luaspob.load
unload = luaspob.unload
can_land = luaspob.can_land
comm = luaspob.comm
