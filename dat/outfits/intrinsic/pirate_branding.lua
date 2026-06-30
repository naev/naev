local pir = require "common.pirate"

require("outfits.lib.custom_price").setup( function ()
   local pp = player.pilot()
   local ps = pp:ship()
   local size = ps:size()
   return 25e3 * math.pow(size, 2)
end )

local odescextra = descextra
function descextra( p, o, po )
   return odescextra( p, o, po ).."\n".._("Ship counts as a pirate ship when applicable.")
end

local REASON_ALREADY_PIRATE = _("Your ship is already branded as pirate.")
local REASON_NOT_CAPTURED = _("Only captured ships can be branded as pirate.")

local oprice = price
function price( q )
   local pricestr, canbuy, cansell, youhave =  oprice( q )
   if pir.isPirateShip( player.pilot() ) then
      return pricestr, REASON_ALREADY_PIRATE, false, youhave
   elseif not player.shipvarPeek( "captured" ) then
      return pricestr, REASON_NOT_CAPTURED, false, youhave
   end
   return pricestr, canbuy, cansell, youhave
end

local obuy = buy
function buy( q )
   if pir.isPirateShip( player.pilot() ) then
      return false, REASON_ALREADY_PIRATE
   elseif not player.shipvarPeek( "captured" ) then
      return false, REASON_NOT_CAPTURED
   end
   return obuy( q )
end
