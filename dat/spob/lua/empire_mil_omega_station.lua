local luaspob = require "spob.lua.lib.spob"

mem.params = {
   std_land = 50,
   std_bribe = 100,
   msg_granted = {
      _("Landing access authorized."),
   },
   msg_notyet = {
      _([["You are not authorized to land here."]]),
   },
   msg_cantbribe = {
      _([["Don't attempt to bribe an Empire official, pilot."]]),
   },
}
luaspob.setup( mem.params )

local function initparams ()
   local std_land = 50
   if player.misnDone("Empire Shipping 3") or player.misnActive("Empire Shipping 3") then
      std_land = 0
   end
   mem.params.std_land = std_land
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
