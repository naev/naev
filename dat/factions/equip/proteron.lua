require "dat/factions/equip/generic.lua"


equip_typeOutfits_coreSystems["Derivative"] = {
   "Milspec Orion 2301 Core System",
}
equip_typeOutfits_coreSystems["Kahan"] = {
   "Milspec Orion 5501 Core System"
}

equip_typeOutfits_engines["Derivative"] = {
   "Tricon Zephyr Engine",
}
equip_typeOutfits_engines["Kahan"] = {
   "Tricon Cyclone II Engine"
}

equip_typeOutfits_hulls["Derivative"] = {
   "S&K Ultralight Combat Plating",
}
equip_typeOutfits_hulls["Kahan"] = {
   "S&K Medium-Heavy Combat Plating"
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
      "Heavy Laser",
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
