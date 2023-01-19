local luaspob = require "spob.lua.lib.spob"

mem.params = {
   std_land = 50,
   std_bribe = 0,
   msg_granted = {
      _("Permisson to land granted."),
   },
   msg_notyet = {
      _("Access to Maanen's Moon denied."),
      _("You need special clearance to land on Maanen's Moon."),
   },
   msg_cantbribe = {
      _([["Don't attempt to bribe an Empire official, pilot."]]),
   },
}

local function initparams ()
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
