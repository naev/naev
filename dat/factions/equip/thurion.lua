require "factions/equip/generic"


equip_typeOutfits_weapons["Perspicacity"] = {
   {
      "Laser Cannon MK1", "Razor MK1", "TeraCom Mace Launcher",
      "TeraCom Banshee Launcher", "Electron Burst Cannon",
   },
}
equip_typeOutfits_weapons["Ingenuity"] = {
   {
      "Laser Cannon MK1", "Razor MK1", "TeraCom Mace Launcher",
      "TeraCom Banshee Launcher", "Electron Burst Cannon",
   },
}
equip_typeOutfits_weapons["Scintillation"] = {
   {
      varied = true;
      "TeraCom Fury Launcher", "TeraCom Medusa Launcher",
      "Unicorp Headhunter Launcher", "Convulsion Launcher",
   },
   {
      "Laser Cannon MK2", "Razor MK2",
   },
}
equip_typeOutfits_weapons["Virtuosity"] = {
   {
      varied = true;
      "Unicorp Fury Launcher", "Unicorp Headhunter Launcher",
      "Unicorp Medusa Launcher", "Unicorp Vengeance Launcher",
      "Enygma Systems Spearhead Launcher", "Unicorp Caesar IV Launcher",
      "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
      "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
      "TeraCom Imperator Launcher", "Convulsion Launcher",
   },
   {
      probability = {
         ["Ripper Cannon"] = 8, ["Slicer"] = 8,
      };
      "Ripper Cannon", "Slicer", "Laser Cannon MK2", "Razor MK2",
   },
}
equip_typeOutfits_weapons["Taciturnity"] = {
   {
      "Heavy Ripper Turret", "Grave Beam", "Heavy Ion Turret",
   },
   {
      num = 1;
      "Enygma Systems Turreted Fury Launcher",
      "Turreted Convulsion Launcher",
   },
   {
      "Laser Turret MK2", "Razor Turret MK2", "Orion Beam",
      "EMP Grenade Launcher", "Enygma Systems Turreted Fury Launcher",
      "Turreted Convulsion Launcher",
   },
}
equip_typeOutfits_weapons["Apprehension"] = {
   {
      "Heavy Ripper Turret", "Grave Beam", "Heavy Ion Turret",
   },
   {
      num = 1;
      "Enygma Systems Turreted Fury Launcher",
      "Turreted Convulsion Launcher",
   },
   {
      "Laser Turret MK2", "Razor Turret MK2", "Orion Beam",
      "EMP Grenade Launcher", "Enygma Systems Turreted Fury Launcher",
      "Turreted Convulsion Launcher",
   },
}
equip_typeOutfits_weapons["Certitude"] = {
   {
      num = 1;
      "Enygma Systems Turreted Fury Launcher",
      "Turreted Convulsion Launcher",
   },
   {
      "Heavy Laser Turret", "Ragnarok Beam",
      "Thurion Perspicacity Fighter Bay",
   },
   {
      "Heavy Ripper Turret", "Grave Beam", "Heavy Ion Turret",
   },
}

equip_typeOutfits_structurals["Perspicacity"] = {
   {
      varied = true, probability = {
         ["Fuel Pod"] = 4, ["Improved Stabilizer"] = 2
      };
      "Fuel Pod", "Improved Stabilizer", "Shield Capacitor",
      "Adaptive Stealth Plating",
   },
}
equip_typeOutfits_structurals["Ingenuity"] = {
   {
      varied = true, probability = {
         ["Improved Stabilizer"] = 4, ["Engine Reroute"] = 4,
      };
      "Fuel Pod", "Improved Stabilizer", "Engine Reroute", "Battery",
      "Shield Capacitor", "Adaptive Stealth Plating",
   },
}
equip_typeOutfits_structurals["Scintillation"] = {
   {
      varied = true;
      "Fuel Pod", "Improved Stabilizer", "Engine Reroute", "Shield Capacitor",
      "Adaptive Stealth Plating",
   },
}
equip_typeOutfits_structurals["Virtuosity"] = {
   {
      varied = true;
      "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
      "Plasteel Plating", "Adaptive Stealth Plating",
   },
}
equip_typeOutfits_structurals["Taciturnity"] = {
   {
      varied = true, probability = {
         ["Cargo Pod"] = 15, ["Medium Fuel Pod"] = 3,
      };
      "Cargo Pod", "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
      "Plasteel Plating", "Adaptive Stealth Plating",
   },
}
equip_typeOutfits_structurals["Apprehension"] = {
   {
      varied = true;
      "Large Fuel Pod", "Battery III", "Shield Capacitor IV",
      "Shield Capacitor III", "Nanobond Plating",
   },
   {
      varied = true;
      "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
      "Plasteel Plating", "Adaptive Stealth Plating",
   },
}


--[[
-- @brief Does Thurion pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   -- They all get the nebula resistant coating
   equip_set( p, {{ "Nebula Resistant Coating", }} )
   equip_generic( p )
end
