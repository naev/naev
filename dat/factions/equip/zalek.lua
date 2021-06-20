require "factions/equip/generic"


equip_typeOutfits_coreSystems["Sting"] = {
   "Milspec Orion 4801 Core System", "Milspec Thalos 4702 Core System",
}
equip_typeOutfits_coreSystems["Demon"] = {
   "Milspec Orion 5501 Core System", "Milspec Thalos 5402 Core System",
}
equip_typeOutfits_coreSystems["Mephisto"] = {
   "Milspec Orion 8601 Core System",
}
equip_typeOutfits_coreSystems["Diablo"] = {
   "Milspec Thalos 9802 Core System",
}
equip_typeOutfits_coreSystems["Hephaestus"] = {
   "Milspec Thalos 9802 Core System",
}

equip_typeOutfits_engines["Sting"] = {
   "Tricon Cyclone Engine",
}
equip_typeOutfits_engines["Demon"] = {
   "Tricon Cyclone II Engine",
}
equip_typeOutfits_engines["Mephisto"] = {
   "Unicorp Eagle 7000 Engine", "Tricon Typhoon II Engine",
}
equip_typeOutfits_engines["Diablo"] = {
   "Tricon Typhoon II Engine",
}
equip_typeOutfits_engines["Hephaestus"] = {
   "Melendez Mammoth XL Engine",
}

equip_typeOutfits_hulls["Sting"] = {
   "Nexus Medium Stealth Plating", "S&K Medium Combat Plating",
}
equip_typeOutfits_hulls["Demon"] = {
   "Nexus Medium Stealth Plating", "S&K Medium-Heavy Combat Plating",
}
equip_typeOutfits_hulls["Mephisto"] = {
   "Unicorp D-48 Heavy Plating", "Unicorp D-68 Heavy Plating",
}
equip_typeOutfits_hulls["Diablo"] = {
   "Unicorp D-48 Heavy Plating", "Unicorp D-68 Heavy Plating",
}
equip_typeOutfits_hulls["Hephaestus"] = {
   "Unicorp D-68 Heavy Plating", "S&K Superheavy Combat Plating",
}

equip_typeOutfits_weapons["Sting"] = {
   {
      probability = {
         ["Za'lek Hunter Launcher"] = 4,
      };
      num = 1;
      varied = true;
      "Enygma Systems Spearhead Launcher", "TeraCom Fury Launcher",
      "TeraCom Headhunter Launcher", "TeraCom Medusa Launcher",
      "TeraCom Vengeance Launcher", "Za'lek Hunter Launcher",
      "Za'lek Reaper Launcher",
   },
   {
      probability = {
         ["Za'lek Light Drone Fighter Bay"] = 3,
         ["Za'lek Bomber Drone Fighter Bay"] = 2,
      };
      varied = true;
      num = 2;
      "Za'lek Heavy Drone Fighter Bay",
      "Za'lek Light Drone Fighter Bay",
      "Za'lek Bomber Drone Fighter Bay",
      "Za'lek Reaper Launcher",
      "Za'lek Hunter Launcher",
   },
   {
      varied = true;
      num = 3,
      "Grave Lance",
      "Orion Lance",
      "Particle Beam",
   },
}
equip_typeOutfits_weapons["Demon"] = {
   {
      "Grave Beam",
   },
   {
      probability = {
         ["Za'lek Light Drone Fighter Bay"] = 3,
         ["Za'lek Bomber Drone Fighter Bay"] = 2,
         ["Za'lek Hunter Launcher"] = 2,
      };
      varied = true;
      "Za'lek Heavy Drone Fighter Bay",
      "Za'lek Light Drone Fighter Bay",
      "Za'lek Bomber Drone Fighter Bay",
      "Za'lek Hunter Launcher",
      "Enygma Systems Turreted Fury Launcher",
      "Enygma Systems Turreted Headhunter Launcher",
      "Grave Lance", "Orion Beam",
   },
}
equip_typeOutfits_weapons["Diablo"] = {
   {
      "Ragnarok Beam",
   },
   {
      "Grave Beam",
   },
   {
      varied = true;
      "Za'lek Light Drone Fighter Dock", "Za'lek Heavy Drone Fighter Dock",
      "Za'lek Bomber Drone Fighter Dock",
   },
}
equip_typeOutfits_weapons["Mephisto"] = {
   {
      "Grave Beam",
   },
}
equip_typeOutfits_weapons["Hephaestus"] = {
   {
      varied = true;
      "Za'lek Light Drone Fighter Dock", "Za'lek Heavy Drone Fighter Dock",
      "Za'lek Bomber Drone Fighter Dock",
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
      varied = true;
      "Droid Repair Crew", "Milspec Scrambler", "Boarding Androids MK1"
   }
}

local zlk_util_medplus = { { varied=true;
      "Faraday Tempest Coating", "Hive Combat AI" } }
equip_shipOutfits_utilities["Za'lek Sting"]     = zlk_util_medplus
equip_shipOutfits_utilities["Za'lek Demon"]     = zlk_util_medplus
equip_shipOutfits_utilities["Za'lek Mephisto"]  = zlk_util_medplus
equip_shipOutfits_utilities["Za'lek Diablo"]    = zlk_util_medplus
equip_shipOutfits_utilities["Za'lek Hephaestus"]= zlk_util_medplus

--[[
-- @brief Does Za'lek pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
