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
   "Unicorp PT-16 Core System", "Milspec Orion 2301 Core System",
}
equip_typeOutfits_coreSystems["Shark"] = equip_typeOutfits_coreSystems["Hyena"]
equip_typeOutfits_coreSystems["Vendetta"] = {
   "Unicorp PT-68 Core System", "Milspec Orion 3701 Core System",
   "Milspec Thalos 3602 Core System",
}
equip_typeOutfits_coreSystems["Lancelot"] = equip_typeOutfits_coreSystems["Vendetta"]
equip_typeOutfits_coreSystems["Ancestor"] = equip_typeOutfits_coreSystems["Vendetta"]
equip_typeOutfits_coreSystems["Phalanx"] = {
   "Unicorp PT-200 Core System", "Milspec Orion 4801 Core System",
}
equip_typeOutfits_coreSystems["Admonisher"] = equip_typeOutfits_coreSystems["Phalanx"]
equip_typeOutfits_coreSystems["Pacifier"] = {
   "Unicorp PT-310 Core System", "Milspec Orion 5501 Core System",
}
equip_typeOutfits_coreSystems["Kestrel"] = {
   "Unicorp PT-2200 Core System", "Milspec Orion 8601 Core System",
}

equip_typeOutfits_engines["Rhino"] = {
   "Unicorp Falcon 1300 Engine", "Tricon Cyclone II Engine",
}

equip_typeOutfits_hulls["Shark"] = {
   "Unicorp D-2 Light Plating", "Nexus Light Stealth Plating",
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

-- TODO add Low Radiation Sensor Modulator when stealth is added to pilots
local pir_util_medplus = { { varied = true;
   "Scanning Combat AI" } }
equip_shipOutfits_utilities["Pirate Phalanx"]   = pir_util_medplus
equip_shipOutfits_utilities["Pirate Admonisher"]= pir_util_medplus
equip_shipOutfits_utilities["Pirate Rhino"]     = pir_util_medplus
equip_shipOutfits_utilities["Pirate Kestrel"]   = pir_util_medplus


--[[
-- @brief Does pirate pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
