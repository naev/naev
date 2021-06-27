require "factions/equip/generic"


equip_typeOutfits_coreSystems["Derivative"] = {
   "Milspec Orion 2301 Core System",
}
equip_typeOutfits_coreSystems["Kahan"] = {
   "Milspec Orion 5501 Core System"
}
equip_typeOutfits_coreSystems["Archimedes"] = {
   "Milspec Orion 9901 Core System"
}
equip_typeOutfits_coreSystems["Watson"] = {
   "Milspec Orion 9901 Core System"
}

equip_typeOutfits_engines["Derivative"] = {
   "Tricon Zephyr Engine",
}
equip_typeOutfits_engines["Kahan"] = {
   "Tricon Cyclone II Engine",
}
equip_typeOutfits_engines["Archimedes"] = {
   "Tricon Typhoon II Engine",
}
equip_typeOutfits_engines["Watson"] = {
   "Melendez Mammoth XL Engine",
}

equip_typeOutfits_hulls["Derivative"] = {
   "Nexus Light Stealth Plating",
}
equip_typeOutfits_hulls["Kahan"] = {
   "Nexus Medium Stealth Plating",
}
equip_typeOutfits_hulls["Archimedes"] = {
   "S&K Superheavy Combat Plating",
}
equip_typeOutfits_hulls["Watson"] = {
   "S&K Superheavy Combat Plating",
}

equip_typeOutfits_weapons["Derivative"] = {
   {
      num = 1;
      "Unicorp Banshee Launcher", "TeraCom Banshee Launcher",
   },
   {
      "Laser Cannon MK1", "Plasma Blaster MK1",
   }
}
equip_typeOutfits_weapons["Kahan"] = {
   {
      num = 2;
      "Railgun", "Heavy Razor Turret", "Grave Beam",
   },
   {
      "Enygma Systems Turreted Fury Launcher",
      "Enygma Systems Turreted Headhunter Launcher",
   }
}
equip_typeOutfits_weapons["Archimedes"] = {
   {
      num = 1;
      "Enygma Systems Turreted Fury Launcher",
      "Enygma Systems Turreted Headhunter Launcher",
   },
   {
      "Heavy Ripper Turret", "Railgun Turret", "Ragnarok Beam",
   },
   {
      "Heavy Razor Turret", "Grave Beam",
   },
}
equip_typeOutfits_weapons["Watson"] = {
   {
      num = 2;
      "Heavy Laser Turret", "Railgun Turret", "Ragnarok Beam",
   },
   {
      "Proteron Derivative Fighter Bay",
   },
   {
      "Heavy Razor Turret", "Grave Beam",
   },
}


--[[
-- @brief Does Proteron pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
