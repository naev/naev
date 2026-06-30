--[[

   Trader Common Functions

--]]
local vn = require "vn"
local traderfct = require "factions.trader_society"

local trader = {}

trader.colour = "#b"
trader.prefix = trader.colour.._("TRADER: ").."#0"

trader.vrata_steve = {
   portrait = "vrata/unique/yearning_steve",
   image = "vrata/unique/yearning_steve",
   name = _("Steve"),
   colour = nil,
   description = _("TODO"),
}

-- Anais Winslow
trader.astra_anais = {
   portrait = "", -- TODO
   image = "", -- TODO
   name = _("Anais"),
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

-- Anais Winslow
function trader.vn_vrata_anais( params )
   return vn.Character.new( trader.vrata_anais.name,
         tmerge( {
            image=trader.vrata_anais.image,
            colour=trader.vrata_anais.colour,
         }, params) )
end

function trader.reputation_max ()
   return var.peek( traderfct.rep_max_var ) or traderfct.rep_max
end

-- Function for adding log entries for miscellaneous one-off missions.
function trader.addMiscLog( text )
   shiplog.create( "trader_misc", _("Miscellaneous"), _("Trader") )
   shiplog.append( "trader_misc", text )
end
function trader.addVrataLog( text )
   shiplog.create( "trader_vrata", _("Mining Vrata"), _("Trader") )
   shiplog.append( "trader_vrata", text )
end
function trader.addAstraVigilisLog( text )
   shiplog.create( "trader_astra_vigilis", _("Astra Vigilis"), _("Trader") )
   shiplog.append( "trader_astra_vigilis", text )
end

return trader
