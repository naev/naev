local obelisk = require "spob.lua.lib.obelisk"
local fmt = require "format"

local reward = outfit.get("Cleansing Flames")

function init( spb )
   local description = function ()
      if player.outfitNum( reward ) > 0 then
         return _("You already have acquired a flow ability from the Obelisk, however, you can activate it to retry the test if you wish.")
      else
         return fmt.f(_("You will be able to acquire the {reward} ability by passing the Obelisk's Test."), {reward=reward} )
      end
   end
   return obelisk.init( spb, "Test of Purification", description, function ()
      local fct = faction.get("Sirius")
      local minstanding = 30
      local curstanding = fct:reputationGlobal()
      if curstanding < minstanding then
         return false, fmt.f(_("You need at least {standing} with {fct} (you have {current})."),
            {standing=minstanding,fct=fct:longname(),current=fmt.number(curstanding)})
      end
      return true, ""
   end )
end

-- Set up globals
can_land = obelisk.can_land
comm = obelisk.comm
