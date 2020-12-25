require("factions/equip/generic")


equip_typeOutfits_coreSystems["Hyena"] = {
   "Unicorp PT-100 Core System", "Milspec Prometheus 2203 Core System"
}
equip_typeOutfits_coreSystems["Shark"] = equip_typeOutfits_coreSystems["Hyena"]
equip_typeOutfits_coreSystems["Vendetta"] = {
   "Unicorp PT-200 Core System", "Milspec Prometheus 3603 Core System"
}
equip_typeOutfits_coreSystems["Lancelot"] = equip_typeOutfits_coreSystems["Vendetta"]
equip_typeOutfits_coreSystems["Ancestor"] = equip_typeOutfits_coreSystems["Vendetta"]
equip_typeOutfits_coreSystems["Phalanx"] = {
   "Unicorp PT-500 Core System", "Milspec Prometheus 4703 Core System"
}
equip_typeOutfits_coreSystems["Admonisher"] = equip_typeOutfits_coreSystems["Phalanx"]
equip_typeOutfits_coreSystems["Pacifier"] = {
   "Unicorp PT-600 Core System", "Milspec Prometheus 5403 Core System"
}
equip_typeOutfits_coreSystems["Kestrel"] = {
   "Unicorp PT-1000 Core System", "Milspec Prometheus 8503 Core System"
}

equip_typeOutfits_engines["Rhino"] = {
   "Unicorp Falcon 1200 Engine", "Nexus Arrow 1200 Engine",
   "Tricon Cyclone II Engine"
}

equip_typeOutfits_weapons["Hyena"] = {
   {
      varied = true;
      "Ion Cannon", "Ripper Cannon", "Plasma Blaster MK1",
      "Plasma Blaster MK2", "Gauss Gun", "Vulcan Gun", "Laser Cannon MK2",
      "Laser Cannon MK3", "Razor MK2"
   }
}
equip_typeOutfits_weapons["Shark"] = {
   {
      varied = true;
      "Ion Cannon", "Ripper Cannon", "Plasma Blaster MK2", "Vulcan Gun",
      "Shredder", "Razor MK2", "Razor MK3", "Unicorp Mace Launcher",
      "Unicorp Banshee Launcher"
   }
}
equip_typeOutfits_weapons["Vendetta"] = equip_typeOutfits_weapons["Shark"]
equip_typeOutfits_weapons["Lancelot"] = {
   {
      varied = true;
      "Ion Cannon", "Ripper Cannon", "Plasma Blaster MK2", "Vulcan Gun",
      "Shredder", "Razor MK2", "Razor MK3", "Unicorp Mace Launcher",
      "Unicorp Banshee Launcher", "Unicorp Medusa Launcher",
      "TeraCom Medusa Launcher", "Heavy Ion Cannon", "Unicorp Fury Launcher",
      "Unicorp Headhunter Launcher"
   }
}
equip_typeOutfits_weapons["Ancestor"] = {
   {
      num = 3, varied = true;
      "Unicorp Mace Launcher", "Unicorp Medusa Launcher",
      "TeraCom Medusa Launcher", "Heavy Ion Cannon", "Unicorp Fury Launcher",
      "Unicorp Headhunter Launcher", "Unicorp Banshee Launcher"
   },
   {
      "Vulcan Gun", "Shredder", "Ripper Cannon", "Plasma Blaster MK2",
      "Ion Cannon", "Unicorp Mace Launcher"
   }
}
equip_typeOutfits_weapons["Rhino"] = {
   {
      varied = true;
      "Pulse Beam", "Heavy Ripper Turret", "Laser Turret MK2",
      "EMP Grenade Launcher", "Enygma Systems Turreted Fury Launcher",
      "Heavy Ion Turret", "Razor Turret MK2"
   }
}
equip_typeOutfits_weapons["Phalanx"] = {
   {
      varied = true;
      "TeraCom Fury Launcher", "Unicorp Headhunter Launcher",
      "TeraCom Medusa Launcher", "Unicorp Medusa Launcher", "Heavy Ion Cannon",
      "Mass Driver MK1", "Mass Driver MK2", "Plasma Turret MK2",
      "Plasma Blaster MK2", "Laser Turret MK1", "Shredder", "Ion Cannon",
      "Heavy Ripper Cannon", "Ripper Cannon"
   }
}
equip_typeOutfits_weapons["Admonisher"] = equip_typeOutfits_weapons["Phalanx"]
equip_typeOutfits_weapons["Pacifier"] = equip_typeOutfits_weapons["Phalanx"]
equip_typeOutfits_weapons["Kestrel"] = {
   {
      varied = true;
      "Repeating Railgun", "Railgun", "Heavy Laser", "Railgun Turret",
      "Laser Turret MK3", "Heavy Ion Turret", "EMP Grenade Launcher",
      "Mass Driver MK2", "Mass Driver MK3", "Heavy Ion Cannon",
      "Turreted Vulcan Gun", "Orion Beam", "Unicorp Caesar IV Launcher",
      "TeraCom Medusa Launcher", "Razor Turret MK3"
   }
}


--[[
-- @brief Does pirate pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
