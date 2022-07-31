--[[
   Scary War Paint

   Makes the player feared by pirates and bounty hunters.
--]]
local fmt = require "format"
local pir

function onload( _o )
   pir = require "common.pirate"
end

function init( p, po )
   p:effectAdd("Paint", math.huge)
end

function onremove( p, po )
   p:effectRm("Paint")
end

function onscanned( p, po, scanner )
   if pir.factionIsPirate(scanner:faction()) then
      scanner:setFriendly(p)
   elseif scanner:name() == _("Bounty Hunter") then
	-- unfortunately, bounty hunters don't scan yet I think
      scanner:setHostile(p)
   end
end

