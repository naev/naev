local obelisk = require "spob.lua.lib.obelisk"
local fmt = require "format"

local reward = outfit.get("Seeking Chakra")

function init( spb )
   local description
   if player.numOutfit( reward ) > 0 then
      description = _("You already have acquired a flow ability from the Obelisk, however, you can activate it to retry the test if you wish.")
   else
      description = fmt.f(_("You will be able to acquire the {reward} ability by passing the Obelisk's Test."), reward )
   end
   return obelisk.init( spb, "Test of Enlightenment", description, function ()
      return true, ""
   end )
end

-- Set up globals
can_land = obelisk.can_land
comm = obelisk.comm
