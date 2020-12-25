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

equip_typeOutfits_engines["Derivative"] = {
   "Tricon Zephyr Engine",
}
equip_typeOutfits_engines["Kahan"] = {
   "Tricon Cyclone II Engine",
}
equip_typeOutfits_engines["Archimedes"] = {
   "Tricon Typhoon II Engine",
}

equip_typeOutfits_hulls["Derivative"] = {
   "S&K Ultralight Combat Plating",
}
equip_typeOutfits_hulls["Kahan"] = {
   "S&K Medium-Heavy Combat Plating",
}
equip_typeOutfits_hulls["Archimedes"] = {
   "Unicorp B-20 Heavy Plating", "S&K Superheavy Combat Plating",
}

equip_typeOutfits_weapons["Derivative"] = {
   {
      num = 1;
      "Unicorp Banshee Launcher", "Ripper Cannon",
   },
   {
      "Ripper Cannon", "Laser Cannon MK3",
   }
}
equip_typeOutfits_weapons["Kahan"] = {
   {
      num = 1;
      "Turbolaser", "Railgun Turret", "Heavy Laser",
   },
   {
      num = 1;
      "Ragnarok Beam", "Grave Beam", "Turbolaser", "Railgun Turret",
      "Heavy Laser", "TeraCom Imperator Launcher",
   },
   {
      "Enygma Systems Turreted Fury Launcher",
      "Enygma Systems Turreted Headhunter Launcher",
   }
}
equip_typeOutfits_weapons["Archimedes"] = {
   {
      num = 1;
      "Heavy Laser", "Heavy Ripper Turret",
   },
   {
      num = 2;
      "Turbolaser", "Railgun Turret", "Heavy Laser",
   },
   {
      num = 2;
      "Ragnarok Beam", "Grave Beam", "Turbolaser", "Railgun Turret",
   },
   {
      "Enygma Systems Turreted Fury Launcher",
      "Enygma Systems Turreted Headhunter Launcher",
   }
}


--[[
-- @brief Does Proteron pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
