-- Generic equipping routines, helper functions and outfit definitions.
include("dat/factions/equip/generic.lua")


equip_classOutfits_weapons["Yacht"] = {
   { "Asterokiller Gauss Gun" }
}

equip_classOutfits_utilities["Yacht"] = {
   { "Asteroid Scanner" }
}

equip_classOutfits_weapons["Courier"] = {
   { "Asterokiller Gauss Gun" }
}

equip_classOutfits_utilities["Courier"] = {
   { "Asteroid Scanner" }
}

equip_classOutfits_weapons["Freighter"] = {
   { "Asterokiller Gauss Gun" }
}

equip_classOutfits_utilities["Freighter"] = {
   { "Asteroid Scanner" }
}


--[[
-- @brief Does miner pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
