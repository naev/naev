local luaspob = require "spob.lua.lib.spob"

function load( spb )
   local std_land = 50
   if player.misnDone("Empire Shipping 3") or player.misnActive("Empire Shipping 3") then
      std_land = 0
   end
   return luaspob.setup( spb, {
      std_land = std_land,
      std_bribe = 100,
      msg_granted = {
         _("Landing access authorized."),
      },
      msg_notyet = {
         _("You are not authorized to land here."),
      },
      msg_cantbribe = {
         _("Don't attempt to bribe an Empire official, pilot."),
      },
   } )
end

can_land = luaspob.can_land
comm = luaspob.comm
