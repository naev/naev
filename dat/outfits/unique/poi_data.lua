notactive = true -- Doesnt' become active

local fmt = require "format"
local poi = require "common.poi"

local COST = 2

function price( _q )
   local canbuy = (COST <= poi.data_get())
   return fmt.f(n_("{num} Encrypted Data Matrix",
                   "{num} Encrypted Data Matrices", COST),{num=COST}), canbuy, false
end

function buy( _q )
   if COST > poi.data_get() then
      return false, fmt.f(_("You do not have enough {item} to purchase this item."),{item=_("Encrypted Data Matrices")})
   end

   poi.data_take( COST )
   return true, 1 -- Can only buy one at a time
end

function sell( _q )
   return false, _("You can not sell this outfit.")
end
