local luaspob = require "spob.lua.lib.spob"

function init( spb )
   return luaspob.init( spb, {
      std_land = 50,
      std_bribe = 100,
      msg_granted = {
         _("Permission to land granted."),
      },
      msg_notyet = {
         _([["Permission denied. You're not truly one of us."]]),
      },
      msg_cantbribe = {
         _([["We don't need your money, outsider."]]),
      },
   } )
end

load = luaspob.load
unload = luaspob.unload
can_land = luaspob.can_land
comm = luaspob.comm
