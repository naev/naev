local luaspob = require "spob.lua.lib.spob"

local params = {
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

local lua_spb
local function initparams ()
   local std_land = 50
   if player.misnDone("Empire Shipping 3") or player.misnActive("Empire Shipping 3") then
      std_land = 0
   end
   params.std_land = std_land
   return luaspob.init( lua_spb, params )
end

function init( spb )
   lua_spb = spb
   return initparams()
end

load = function ()
   initparams()
   return luaspob.load()
end
unload = luaspob.unload
can_land = luaspob.can_land
comm = luaspob.comm
