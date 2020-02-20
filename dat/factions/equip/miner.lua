-- Generic equipping routines, helper functions and outfit definitions.
include("dat/factions/equip/generic.lua")


equip_classOutfits_weapons["Yacht"] = {
   {
      "Laser Cannon MK1", "Gauss Gun"
   }
}

equip_classOutfits_weapons["Courier"] = {
   {
      "Turreted Gauss Gun", "Laser PD MK2"
   }
}

equip_classOutfits_weapons["Freighter"] = {
   {
      num = 1;
      "Laser Turret MK2", "Plasma Turret MK2",
      "Enygma Systems Turreted Fury Launcher"
   },
   {
      "Laser Turret MK2", "Plasma Turret MK2"
   }
}


--[[
-- @brief Does miner pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
