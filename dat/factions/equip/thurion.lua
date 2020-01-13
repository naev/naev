include("dat/factions/equip/generic.lua")


equip_shipOutfits_weapons["Thurion Perspicacity"] = {
   {
      "Energy Dart", "Ripper Cannon"
   }
}
equip_shipOutfits_weapons["Thurion Ingenuity"] = {
   {
      varied = true;
      "Energy Dart", "Ripper Cannon", "Ion Cannon", "Laser Cannon MK3"
   }
}
equip_shipOutfits_weapons["Thurion Scintillation"] = {
   {
      varied = true;
      "Convulsion Launcher", "Energy Missile", "Unicorp Medusa Launcher"
   }
}
equip_shipOutfits_weapons["Thurion Virtuosity"] = {
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
equip_shipOutfits_weapons["Thurion Taciturnity"] = {
   {
      num = 2, varied = true;
      "Turreted Convulsion Launcher", "EMP Grenade", "Laser Turret MK1"
   },
   {
      varied = true;
      "Turreted Gauss Gun", "Laser PD MK1"
   }
}
equip_shipOutfits_weapons["Goddard"] = {
   {
      varied = true;
      "Railgun", "Energy Torpedo", "Turreted Convulsion Launcher"
   }
}

equip_shipoutfits_utilities["Thurion Perspicacity"] = {
   {
      varied = true;
      "Unicorp Scrambler", "Sensor Array", "Jump Scanner",
      "Thurion Reactor Class I", "Small Shield Booster", "Generic Afterburner"
   }
}
equip_shipoutfits_utilities["Thurion Ingenuity"] = {
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
equip_shipoutfits_utilities["Thurion Scintillation"] = {
   {
      varied = true;
      "Sensor Array", "Unicorp Scrambler", "Thurion Reactor Class I",
      "Small Shield Booster", "Hellburner"
   }
}
equip_shipoutfits_utilities["Thurion Virtuosity"] = {
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
equip_shipoutfits_utilities["Thurion Taciturnity"] = {
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
equip_shipoutfits_utilities["Goddard"] = {
   {
      varied = true;
      "Boarding Androids MKII", "Thurion Reactor Class III",
      "Large Shield Booster", "Droid Repair Crew", "Sensor Array",
      "Milspec Scrambler"
   }
}

equip_shipoutfits_structurals["Thurion Perspicacity"] = {
   {
      varied = true;
      "Adaptive Stealth Plating", "Fuel Pod", "Thurion Engine Reroute",
      "Steering Thrusters", "Shield Capacitor"
   }
}
equip_shipoutfits_structurals["Thurion Ingenuity"] = {
   {
      varied = true;
      "Adaptive Stealth Plating", "Fuel Pod", "Thurion Engine Reroute",
      "Steering Thrusters", "Shield Capacitor"
   }
}
equip_shipoutfits_structurals["Thurion Scintillation"] = {
   {
      varied = true;
      "Adaptive Stealth Plating", "Fuel Pod", "Thurion Engine Reroute",
      "Steering Thrusters", "Shield Capacitor"
   }
}
equip_shipoutfits_structurals["Thurion Virtuosity"] = {
   {
      varied = true;
      "Active Plating", "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
      "Adaptive Stealth Plating"
   }
}
equip_shipoutfits_structurals["Thurion Taciturnity"] = {
   {
      varied = true;
      "Active Plating", "Medium Fuel Pod", "Battery II", "Shield Capacitor II",
      "Adaptive Stealth Plating", "Cargo Pod"
   }
}
equip_shipoutfits_structurals["Goddard"] = {
   {
      num = 2;
      "Nebular Shielding Prototype"
   },
   {
      "Biometal Armour", "Nanobond Plating", "Large Fuel Pod", "Battery III",
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
