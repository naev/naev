require("dat/factions/equip/generic.lua")


equip_typeOutfits_coreSystems["Fidelity"] = {
   "Milspec Prometheus 2203 Core System"
}
equip_typeOutfits_coreSystems["Shaman"] = {
   "Milspec Prometheus 3603 Core System"
}
equip_typeOutfits_coreSystems["Preacher"] = {
   "Milspec Orion 4801 Core System"
}
equip_typeOutfits_coreSystems["Dogma"] = {
   "Milspec Orion 9901 Core System"
}
equip_typeOutfits_coreSystems["Divinity"] = equip_typeOutfits_coreSystems["Dogma"]

equip_typeOutfits_engines["Fidelity"] = {
   "Tricon Zephyr Engine"
}
equip_typeOutfits_engines["Shaman"] = {
   "Tricon Zephyr II Engine"
}
equip_typeOutfits_engines["Preacher"] = {
   "Tricon Cyclone Engine"
}
equip_typeOutfits_engines["Dogma"] = {
   "Tricon Typhoon II Engine", "Nexus Bolt 6500 Engine"
}
equip_typeOutfits_engines["Divinity"] = {
   "Melendez Mammoth XL Engine"
}

equip_typeOutfits_hulls["Fidelity"] = {
   "S&K Ultralight Stealth Plating", "S&K Ultralight Combat Plating"
}
equip_typeOutfits_hulls["Shaman"] = {
   "S&K Light Combat Plating"
}
equip_typeOutfits_hulls["Preacher"] = {
   "S&K Medium Combat Plating"
}
equip_typeOutfits_hulls["Dogma"] = {
   "Unicorp B-20 Heavy Plating", "S&K Superheavy Combat Plating"
}
equip_typeOutfits_hulls["Divinity"] = {
   "S&K Superheavy Combat Plating"
}

equip_typeOutfits_weapons["Fidelity"] = equip_shipOutfits_weapons["Sirius Fidelity"]
equip_typeOutfits_weapons["Shaman"] = {
   {
      num = 1;
      "Heavy Ion Cannon", "Unicorp Medusa Launcher", "TeraCom Medusa Launcher",
      "Enygma Systems Spearhead Launcher", "Unicorp Headhunter Launcher"
   },
   {
      num = 1;
      "Razor MK3", "Ion Cannon"
   },
   {
      "Razor MK2", "Razor MK3", "Ion Cannon"
   }
}
equip_typeOutfits_weapons["Preacher"] = {
   {
      num = 2;
      "Heavy Ion Cannon", "Razor MK3", "Shattershield Lance"
   },
   {
      num = 2, varied = true;
      "Heavy Ion Cannon", "Unicorp Medusa Launcher", "TeraCom Medusa Launcher",
      "Enygma Systems Spearhead Launcher", "Unicorp Headhunter Launcher"
   },
   {
      varied = true;
      "Heavy Ion Cannon", "Razor MK3", "Razor Turret MK2", "Razor Turret MK3",
      "Laser Turret MK1", "Laser Turret MK2"
   }
}
equip_typeOutfits_weapons["Dogma"] = {
   {
      varied = true;
      "Heavy Razor Turret", "Heavy Ion Cannon", "Razor Turret MK3"
   }
}
equip_typeOutfits_weapons["Divinity"] = equip_typeOutfits_weapons["Dogma"] --[[ {
   {
      num = 1;
      "Fidelity Fighter Bay"
   },
   {
      num = 2;
      "Heavy Razor Turret"
   },
   {
      "Fidelity Fighter Bay"
   }
}
--]]


--[[
-- @brief Does sirius pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
