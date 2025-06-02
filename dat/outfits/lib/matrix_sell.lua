notactive = true

local fmt = require "format"
local poi = require "common.poi"

local COST = 1 -- matrices

local PRICE
function onload( o )
   PRICE = o:price()
end

-- TODO something more robust than this
local store_name = _("Machiavellian Misi's \"Fine\" Wares")
local function is_misi()
   return player.merchantOutfitName() == store_name
end

function price( q )
   if is_misi() then
      local playerhas = poi.data_get()
      return fmt.f(n_("{num} Encrypted Data Matrix",
                     "{num} Encrypted Data Matrices", q*COST),{num=q*COST}),
               true, true,
               fmt.f(n_("{num} Encrypted Data Matrix",
                     "{num} Encrypted Data Matrices", playerhas),{num=playerhas})
   end
   return fmt.credits(q*PRICE), true, true, fmt.credits(player.credits())
end

function buy( q )
   if is_misi() then
      if q*COST > poi.data_get() then
         return false, fmt.f(_("You do not have enough {item} to purchase this item."),{item=_("Encrypted Data Matrices")})
      end
      poi.data_take( q*COST )
      return true, q
   end
   if q*PRICE > player.credits() then
      return false, fmt.f(_("You need {credits} more."), {credits=fmt.credits(q*PRICE-player.credits())} )
   end
   player.pay( -q*PRICE )
   return true, q
end

function sell( q )
   if is_misi() then
      poi.data_give( q*COST )
      return true, q
   end
   player.pay( q*PRICE )
   return true, q
end
