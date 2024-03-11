local obelisk = require "spob.lua.lib.obelisk"
local fmt = require "format"

local reward = outfit.get("Astral Projection")
local reqs = {
   outfit.get("Feather Drive"),
   outfit.get("Seeking Chakra"),
}

function init( spb )
   local description
   if player.outfitNum( reward ) > 0 then
      description = _("You already have acquired a flow ability from the Obelisk, however, you can activate it to retry the test if you wish.")
   else
      description = fmt.f(_("You will be able to acquire the {reward} ability by passing the Obelisk's Test."), {reward=reward} )
   end
   return obelisk.init( spb, "Test of Renewal", description, function ()
      for k,o in ipairs(reqs) do
         if player.outfitNum( o ) <= 0 then
            local desc = _("You need the following flow abilities to be able to activate the Obelisk:")
            for i,r in ipairs(reqs) do
               if player.outfitNum( r ) <= 0 then
                  desc = desc.."\n#r"..r:name().."#0"
               else
                  desc = desc.."\n"..r:name()
               end
            end
            return false, desc
         end
      end
      local fct = faction.get("Sirius")
      local minstanding = 10
      local curstanding = fct:playerStanding()
      if curstanding < minstanding then
         return false, fmt.f(_("You need at least {standing} with {fct} (you have {current})."),
            {standing=minstanding,fct=fct:longname(),current=curstanding})
      end
      return true, ""
   end )
end

-- Set up globals
can_land = obelisk.can_land
comm = obelisk.comm
