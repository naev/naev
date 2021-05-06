require "factions/equip/generic"


equip_typeOutfits_coreSystems["Sting"] = {
   "Milspec Aegis 4701 Core System",
}
equip_typeOutfits_coreSystems["Demon"] = {
   "Milspec Prometheus 5403 Core System",
}
equip_typeOutfits_coreSystems["Mephisto"] = {
   "Milspec Prometheus 8503 Core System",
}
equip_typeOutfits_coreSystems["Diablo"] = {
   "Milspec Orion 9901 Core System",
}
equip_typeOutfits_coreSystems["Hephaestus"] = {
   "Milspec Hermes 9802 Core System",
}
equip_typeOutfits_coreSystems["Prototype"] = {
   "Milspec Orion 9901 Core System",
}

equip_typeOutfits_engines["Sting"] = {
   "Tricon Cyclone Engine",
}
equip_typeOutfits_engines["Demon"] = {
   "Tricon Cyclone II Engine",
}
equip_typeOutfits_engines["Mephisto"] = {
   "Unicorp Eagle 6500 Engine", "Tricon Typhoon II Engine",
}
equip_typeOutfits_engines["Diablo"] = {
   "Tricon Typhoon II Engine",
}
equip_typeOutfits_engines["Hephaestus"] = {
   "Melendez Mammoth XL Engine",
}
equip_typeOutfits_engines["Prototype"] = {
   "Unicorp Eagle 6500 Engine", "Tricon Typhoon II Engine",
}

equip_typeOutfits_hulls["Sting"] = {
   "S&K Medium Stealth Plating", "S&K Medium Combat Plating",
}
equip_typeOutfits_hulls["Demon"] = {
   "S&K Medium-Heavy Stealth Plating", "S&K Medium-Heavy Combat Plating",
}
equip_typeOutfits_hulls["Mephisto"] = {
   "Unicorp D-48 Heavy Plating", "S&K Heavy Combat Plating",
}
equip_typeOutfits_hulls["Diablo"] = {
   "Unicorp D-48 Heavy Plating", "S&K Heavy Combat Plating",
}
equip_typeOutfits_hulls["Hephaestus"] = {
   "Unicorp D-72 Heavy Plating", "S&K Superheavy Combat Plating",
}
equip_typeOutfits_hulls["Prototype"] = {
   "Unicorp D-72 Heavy Plating", "S&K Superheavy Combat Plating",
}

equip_typeOutfits_weapons["Sting"] = {
   {
      varied = true;
      "Enygma Systems Spearhead Launcher", "TeraCom Fury Launcher",
      "TeraCom Headhunter Launcher", "TeraCom Medusa Launcher",
      "TeraCom Vengeance Launcher", "Za'lek Hunter Launcher",
      "Za'lek Reaper Launcher",
   },
   {
      "Orion Lance",
   },
}
equip_typeOutfits_weapons["Demon"] = {
   {
      "Grave Beam",
   },
   {
      num = 1;
      "Enygma Systems Turreted Fury Launcher",
      "Enygma Systems Turreted Headhunter Launcher",
   },
   {
      num = 1;
      "Enygma Systems Turreted Fury Launcher",
      "Enygma Systems Turreted Headhunter Launcher",
      "Grave Lance", "Orion Beam",
   },
   {
      "Grave Lance", "Orion Beam",
   },
}
equip_typeOutfits_weapons["Diablo"] = {
   {
      varied = true;
      "Za'lek Light Drone Fighter Bay", "Za'lek Heavy Drone Fighter Bay",
      "Za'lek Bomber Drone Fighter Bay",
   },
   {
      "Ragnarok Beam",
   },
   {
      "Grave Beam",
   },
}
equip_typeOutfits_weapons["Mephisto"] = {
   {
      num = 1;
      "Enygma Systems Turreted Fury Launcher",
      "Enygma Systems Turreted Headhunter Launcher",
      "Za'lek Hunter Launcher",
   },
   {
      "Grave Beam",
   },
}
equip_typeOutfits_weapons["Hephaestus"] = {
   {
      varied = true;
      "Za'lek Light Drone Fighter Bay", "Za'lek Heavy Drone Fighter Bay",
      "Za'lek Bomber Drone Fighter Bay",
   },
   {
      "Ragnarok Beam",
   },
   {
      "Grave Beam",
   },
}

equip_typeOutfits_utilities["Demon"] = {
   {
      num = 1;
      "Reactor Class III"
   },
   {
      num = 1;
      "Large Shield Booster"
   },
   {
      varied = true;
      "Droid Repair Crew", "Milspec Scrambler", "Boarding Androids MK1"
   }
}


--[[
-- @brief Does Za'lek pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
