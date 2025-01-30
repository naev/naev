--[[
   Scary War Paint

   Makes the player feared by pirates and bounty hunters.
--]]
local pir
notactive = true

function onload( _o )
   pir = require "common.pirate"
end

function init( p, _po )
   p:effectAdd("Scary Paint")
   mem.isp = (p==player.pilot())
end

function onremove( p, _po )
   p:effectRm("Scary Paint")
end

-- TODONOTE: when independents start scanning, change the else branch!
function onscanned( p, _po, scanner )
   if pir.factionIsPirate(scanner:faction()) then
      if pir.maxClanStanding() > 20 then
         scanner:setFriendly(p)
      else
         scanner:setHostile(p, false)
      end
   else
      if mem.isp and not scanner:withPlayer() then
         scanner:setHostile(p)
      end
   end
end
