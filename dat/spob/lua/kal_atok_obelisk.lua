local obelisk = require "spob.lua.lib.obelisk"

function init( spb )
   return obelisk.init( spb, "Test of Enlightenment", function ()
      return true, ""
   end )
end

-- Set up globals
can_land = obelisk.can_land
comm = obelisk.comm
