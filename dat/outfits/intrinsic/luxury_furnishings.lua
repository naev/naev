local lmisn = require "lmisn"
require("outfits.lib.custom_price").setup( function ()
   local pp = player.pilot()
   local ps = pp:ship()
   local size = ps:size()
   local mod = (ps:tags().civilian and 0.5) or 1 -- Civilian ships are 50% cheaper
   return 250e3 * math.pow(1+size, 2) * mod -- 1 million for size 1, 12.25 million for size 6
end )

local odescextra = descextra
function descextra( p, o, po )
   return odescextra( p, o, po ).."\n".._("Ship will count as a luxury ship when applicable.")
end

local REASON_ALREADY_LUXURY = _("Your ship is already a luxury vessel.")

local oprice = price
function price( q )
   local pricestr, canbuy, cansell, youhave =  oprice( q )
   if lmisn.is_luxury( player.pilot() ) then
      return  pricestr, REASON_ALREADY_LUXURY, false, youhave
   end
   return pricestr, canbuy, cansell, youhave
end

local obuy = buy
function buy( q )
   if lmisn.is_luxury( player.pilot() ) then
      return false, REASON_ALREADY_LUXURY
   end
   return obuy( q )
end
