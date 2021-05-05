require "factions/equip/generic"


-- Probability of cargo by class.
equip_classCargo["Yacht"] = .25
equip_classCargo["Luxury Yacht"] = .25
equip_classCargo["Scout"] = .25
equip_classCargo["Courier"] = .25
equip_classCargo["Freighter"] = .25
equip_classCargo["Armoured Transport"] = .25
equip_classCargo["Fighter"] = .25
equip_classCargo["Bomber"] = .25
equip_classCargo["Corvette"] = .25
equip_classCargo["Destroyer"] = .25
equip_classCargo["Cruiser"] = .25
equip_classCargo["Carrier"] = .25
equip_classCargo["Drone"] = .1
equip_classCargo["Heavy Drone"] = .1

equip_classOutfits_weapons["Yacht"] = {
   {
      "Laser Cannon MK1", "Gauss Gun", "Plasma Blaster MK1",
   },
}

equip_classOutfits_weapons["Courier"] = {
   {
      "Laser Turret MK1", "Turreted Gauss Gun", "Plasma Turret MK1",
   },
}

equip_classOutfits_weapons["Freighter"] = {
   {
      num = 1;
      "Enygma Systems Turreted Fury Launcher"
   },
   {
      "Laser Turret MK2", "Turreted Vulcan Gun", "Plasma Turret MK2",
      "Orion Beam",
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
