notactive = true -- Doesn't become active

local fmt = require "format"
local poi = require "common.poi"
local vn = require "vn"
local loot = require "common.loot"

local COST = 3

function price( _q )
   local playerhas = poi.data_get()
   local canbuy = (COST <= playerhas)
   return fmt.f(n_("{num} Encrypted Data Matrix",
                   "{num} Encrypted Data Matrices", COST),{num=COST}),
            canbuy, false,
            fmt.f(n_("{num} Encrypted Data Matrix",
                   "{num} Encrypted Data Matrices", playerhas),{num=playerhas})
end

function buy( _q )
   if COST > poi.data_get() then
      return false, fmt.f(_("You do not have enough {item} to purchase this item."),{item=_("Encrypted Data Matrices")})
   end

   poi.data_take( COST )
   local reward = loot.tier1()
   player.outfitAdd( reward )

   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(fmt.reward(reward))
   vn.run()

   return true, 0 -- Doesn't actually get added
end

function sell( _q )
   return false, _("You can not sell this outfit.")
end
