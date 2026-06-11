local pir = require "common.pirate"

require("outfits.lib.custom_price").setup( function ()
   local pp = player.pilot()
   local ps = pp:ship()
   local size = ps:size()
   return 25e3 * math.pow(size, 2)
end )

function descextra()
   return _("Ship counts as a pirate ship when applicable.")
end

local oprice = price
function price( q )
   local pricestr, canbuy, cansell, youhave =  oprice( q )
   if pir.isPirateShip( player.pilot() ) then
      return pricestr, false, false, youhave
   elseif not player.shipvarPeek( "captured" ) then
      return pricestr, false, false, youhave
   end
   return pricestr, canbuy, cansell, youhave
end

local obuy = buy
function buy( q )
   if pir.isPirateShip( player.pilot() ) then
      return false, _("Your ship is already branded as pirate.")
   elseif not player.shipvarPeek( "captured" ) then
      return false, _("Only captured ships can be branded as pirate.")
   end
   return obuy( q )
end
