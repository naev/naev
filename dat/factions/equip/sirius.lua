include("dat/factions/equip/generic.lua")


equip_shipOutfits_coreSystems["Sirius Fidelity"] = {
   "Milspec Prometheus 2203 Core System"
}
equip_shipOutfits_coreSystems["Sirius Shaman"] = {
   "Milspec Prometheus 3603 Core System"
}
equip_shipOutfits_coreSystems["Sirius Preacher"] = {
   "Milspec Orion 4801 Core System"
}
equip_shipOutfits_coreSystems["Sirius Dogma"] = {
   "Milspec Orion 9901 Core System"
}
equip_shipOutfits_coreSystems["Sirius Divinity"] = {
   "Milspec Orion 9901 Core System"
}

equip_shipOutfits_engines["Sirius Fidelity"] = {
   "Tricon Zephyr Engine"
}
equip_shipOutfits_engines["Sirius Shaman"] = {
   "Tricon Zephyr II Engine"
}
equip_shipOutfits_engines["Sirius Preacher"] = {
   "Tricon Cyclone Engine"
}
equip_shipOutfits_engines["Sirius Dogma"] = {
   "Tricon Typhoon II Engine", "Nexus Bolt 6500 Engine"
}
equip_shipOutfits_engines["Sirius Divinity"] = {
   "Melendez Mammoth XL Engine"
}

equip_shipOutfits_hull["Sirius Fidelity"] = {
   "S&K Ultralight Stealth Plating", "S&K Ultralight Combat Plating"
}
equip_shipOutfits_hull["Sirius Shaman"] = {
   "S&K Light Combat Plating"
}
equip_shipOutfits_hull["Sirius Preacher"] = {
   "S&K Medium Combat Plating"
}
equip_shipOutfits_hull["Sirius Dogma"] = {
   "Unicorp B-20 Heavy Plating", "S&K Superheavy Combat Plating"
}
equip_shipOutfits_hull["Sirius Divinity"] = {
   "S&K Superheavy Combat Plating"
}

-- Sirius Fidelity weapons specified in generic.lua
equip_shipOutfits_weapons["Sirius Shaman"] = {
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
equip_shipOutfits_weapons["Sirius Preacher"] = {
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
equip_shipOutfits_weapons["Sirius Dogma"] = {
   {
      varied = true;
      "Heavy Razor Turret", "Heavy Ion Cannon", "Razor Turret MK3"
   }
}
equip_shipOutfits_weapons["Sirius Divinity"] = {
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


--[[
-- @brief Does sirius pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
