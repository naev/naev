--[[
   Scary War Paint

   Makes the player feared by pirates and bounty hunters.
--]]
local pir

function onload( _o )
   pir = require "common.pirate"
end

function init( p, _po )
   p:effectAdd("Paint", math.huge)
end

function onremove( p, _po )
   p:effectRm("Paint")
end

-- TODONOTE: when independents start scanning, change the else branch!
function onscanned( p, _po, scanner )
   if pir.factionIsPirate(scanner:faction()) then
      if pir.maxClanStanding() > 20 then
         scanner:setFriendly(p)
      else
         scanner.setHostile(p, false)
      end
   else -- if scanner:name() == _("Bounty Hunter") then
	-- unfortunately, bounty hunters don't scan yet I think
      scanner:setHostile(p)
   end
end

