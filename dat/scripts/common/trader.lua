--[[

   Trader Common Functions

--]]
local vn = require "vn"
local traderfct = require "factions.trader_society"

local trader = {}

trader.prefix = "#b".._("TRADER: ").."#0"

trader.vrata_steve = {
   portrait = "vrata/unique/yearning_steve",
   image = "vrata/unique/yearning_steve",
   name = _("Steve"),
   colour = nil,
   description = _("TODO"),
}
function trader.vn_vrata_steve( params )
   return vn.Character.new( trader.vrata_steve.name,
         tmerge( {
            image=trader.vrata_steve.image,
            colour=trader.vrata_steve.colour,
         }, params) )
end

function trader.reputation_max ()
   return var.peek( traderfct.rep_max_var ) or traderfct.rep_max
end

return trader
