require "factions/equip/generic"


-- Probability of cargo by class.
equip_classCargo["Yacht"] = .1
equip_classCargo["Luxury Yacht"] = .1
equip_classCargo["Scout"] = .1
equip_classCargo["Courier"] = .1
equip_classCargo["Freighter"] = .1
equip_classCargo["Armored Transport"] = .1
equip_classCargo["Fighter"] = .1
equip_classCargo["Bomber"] = .1
equip_classCargo["Corvette"] = .1
equip_classCargo["Destroyer"] = .1
equip_classCargo["Cruiser"] = .1
equip_classCargo["Carrier"] = .1
equip_classCargo["Drone"] = .1
equip_classCargo["Heavy Drone"] = .1

equip_classOutfits_coreSystems["Yacht"] = {
   "Unicorp PT-18 Core System", "Milspec Aegis 2201 Core System",
   "Milspec Prometheus 2203 Core System", "Milspec Orion 2301 Core System",
}
equip_classOutfits_engines["Yacht"] = {
   "Nexus Dart 150 Engine", "Tricon Zephyr Engine",
}
equip_classOutfits_hulls["Yacht"] = {
   "Unicorp D-2 Light Plating", "S&K Ultralight Stealth Plating",
   "S&K Ultralight Combat Plating",
}
equip_classOutfits_weapons["Yacht"] = {
   {
      "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
      "Ion Cannon",
   },
}


--[[
-- @brief Does pirate pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
