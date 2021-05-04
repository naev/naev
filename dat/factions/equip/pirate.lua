require "factions/equip/generic"


-- Probability of cargo by class.
equip_classCargo["Yacht"] = .5
equip_classCargo["Luxury Yacht"] = .5
equip_classCargo["Scout"] = .5
equip_classCargo["Courier"] = .5
equip_classCargo["Freighter"] = .5
equip_classCargo["Armoured Transport"] = .5
equip_classCargo["Fighter"] = .5
equip_classCargo["Bomber"] = .5
equip_classCargo["Corvette"] = .5
equip_classCargo["Destroyer"] = .5
equip_classCargo["Cruiser"] = .5
equip_classCargo["Carrier"] = .5
equip_classCargo["Drone"] = .1
equip_classCargo["Heavy Drone"] = .1

equip_typeOutfits_coreSystems["Hyena"] = {
   "Unicorp PT-18 Core System", "Milspec Prometheus 2203 Core System",
}
equip_typeOutfits_coreSystems["Shark"] = equip_typeOutfits_coreSystems["Hyena"]
equip_typeOutfits_coreSystems["Vendetta"] = {
   "Unicorp PT-80 Core System", "Milspec Prometheus 3603 Core System",
}
equip_typeOutfits_coreSystems["Lancelot"] = equip_typeOutfits_coreSystems["Vendetta"]
equip_typeOutfits_coreSystems["Ancestor"] = equip_typeOutfits_coreSystems["Vendetta"]
equip_typeOutfits_coreSystems["Phalanx"] = {
   "Unicorp PT-280 Core System", "Milspec Prometheus 4703 Core System",
}
equip_typeOutfits_coreSystems["Admonisher"] = equip_typeOutfits_coreSystems["Phalanx"]
equip_typeOutfits_coreSystems["Pacifier"] = {
   "Unicorp PT-400 Core System", "Milspec Prometheus 5403 Core System",
}
equip_typeOutfits_coreSystems["Kestrel"] = {
   "Unicorp PT-3400 Core System", "Milspec Prometheus 8503 Core System",
}

equip_typeOutfits_engines["Rhino"] = {
   "Unicorp Falcon 1200 Engine", "Tricon Cyclone II Engine",
}

equip_typeOutfits_hulls["Shark"] = {
   "Unicorp D-2 Light Plating", "S&K Ultralight Stealth Plating",
}

equip_typeOutfits_weapons["Shark"] = {
   {
      num = 2;
      "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
      "Ion Cannon",
   },
   {
      varied = true;
      "Unicorp Banshee Launcher", "TeraCom Banshee Launcher",
      "Unicorp Mace Launcher", "TeraCom Mace Launcher",
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
