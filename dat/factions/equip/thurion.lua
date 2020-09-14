require("dat/factions/equip/generic.lua")


equip_typeOutfits_weapons["Perspicacity"] = {
   {
      "Energy Dart", "Ripper Cannon"
   }
}
equip_typeOutfits_weapons["Ingenuity"] = {
   {
      varied = true;
      "Energy Dart", "Ripper Cannon", "Ion Cannon", "Laser Cannon MK3"
   }
}
equip_typeOutfits_weapons["Scintillation"] = {
   {
      varied = true;
      "Convulsion Launcher", "Energy Missile", "Unicorp Medusa Launcher"
   }
}
equip_typeOutfits_weapons["Virtuosity"] = {
   {
      num = 1;
      "Convulsion Launcher", "Energy Missile"
   },
   {
      num = 1;
      "Energy Dart", "Ripper Cannon"
   },
   {
      varied = true;
      "Energy Dart", "Ripper Cannon", "Ion Cannon", "Laser Cannon MK3"
   }
}
equip_typeOutfits_weapons["Taciturnity"] = {
   {
      num = 2, varied = true;
      "Turreted Convulsion Launcher", "EMP Grenade", "Laser Turret MK1"
   },
   {
      varied = true;
      "Turreted Gauss Gun", "Laser PD MK1"
   }
}
equip_typeOutfits_weapons["Apprehension"] = {
   {
      varied = true;
       "Laser Turret MK3", "Railgun", "Energy Torpedo", "Turreted Convulsion Launcher"
   }
}
equip_typeOutfits_weapons["Certitude"] = {
   {
      varied = true;
      "Railgun", "Energy Torpedo", "Turreted Convulsion Launcher"
   }
}

equip_typeOutfits_utilities["Perspicacity"] = {
   {
      varied = true;
      "Unicorp Scrambler", "Sensor Array", "Jump Scanner",
      "Thurion Reactor Class I", "Small Shield Booster", "Generic Afterburner"
   }
}
equip_typeOutfits_utilities["Ingenuity"] = {
   {
      num = 1;
      "Droid Repair Crew", "Boarding Androids MKI", "Thurion Reactor Class II",
      "Medium Shield Booster"
   },
   {
      varied = true;
      "Sensor Array", "Unicorp Scrambler", "Thurion Reactor Class I",
      "Small Shield Booster", "Hellburner"
   }
}
equip_typeOutfits_utilities["Scintillation"] = {
   {
      varied = true;
      "Sensor Array", "Unicorp Scrambler", "Thurion Reactor Class I",
      "Small Shield Booster", "Hellburner"
   }
}
equip_typeOutfits_utilities["Virtuosity"] = {
   {
      varied = true;
      "Droid Repair Crew", "Boarding Androids MKI", "Thurion Reactor Class II",
      "Medium Shield Booster"
   },
   {
      varied = true;
      "Sensor Array", "Milspec Scrambler", "Thurion Reactor Class I",
      "Small Shield Booster", "Hellburner"
   }
}
equip_typeOutfits_utilities["Taciturnity"] = {
   {
      varied = true;
      "Droid Repair Crew", "Boarding Androids MKI", "Thurion Reactor Class II",
      "Medium Shield Booster"
   },
   {
      varied = true;
      "Sensor Array", "Milspec Scrambler", "Thurion Reactor Class I",
      "Small Shield Booster", "Hellburner"
   }
}
equip_typeOutfits_utilities["Apprehension"] = {
   {
      varied = true;
      "Boarding Androids MKII", "Thurion Reactor Class II",
      "Medium Shield Booster", "Droid Repair Crew", "Sensor Array",
      "Milspec Scrambler"
   }
}
equip_typeOutfits_utilities["Certitude"] = {
   {
      varied = true;
      "Boarding Androids MKII", "Thurion Reactor Class III",
      "Large Shield Booster", "Droid Repair Crew", "Sensor Array",
      "Milspec Scrambler"
   }
}

equip_typeOutfits_structurals["Perspicacity"] = {
   {
      varied = true;
      "Adaptive Stealth Plating", "Fuel Pod", "Thurion Engine Reroute",
      "Steering Thrusters", "Shield Capacitor"
   }
}
equip_typeOutfits_structurals["Ingenuity"] = {
   {
      varied = true;
      "Adaptive Stealth Plating", "Fuel Pod", "Thurion Engine Reroute",
      "Steering Thrusters", "Shield Capacitor"
   }
}
equip_typeOutfits_structurals["Scintillation"] = {
   {
      varied = true;
      "Adaptive Stealth Plating", "Fuel Pod", "Thurion Engine Reroute",
      "Steering Thrusters", "Shield Capacitor"
   }
}
equip_typeOutfits_structurals["Virtuosity"] = {
   {
      varied = true;
      "Active Plating", "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
      "Adaptive Stealth Plating"
   }
}
equip_typeOutfits_structurals["Taciturnity"] = {
   {
      varied = true;
      "Active Plating", "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
      "Adaptive Stealth Plating", "Cargo Pod"
   }
}
equip_typeOutfits_structurals["Apprehension"] = {
   {
      varied = true;
      "Active Plating", "Nanobond Plating", "Medium Fuel Pod", "Battery II",
      "Shield Capacitor III"
   }
}
equip_typeOutfits_structurals["Certitude"] = {
   {
      varied = true;
      "Biometal Armor", "Nanobond Plating", "Large Fuel Pod", "Battery III",
      "Shield Capacitor IV"
   }
}


--[[
-- @brief Does thurion pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
