require "factions/equip/generic"


equip_typeOutfits_coreSystems["Shark"] = {
   "Milspec Orion 2301 Core System",
}
equip_typeOutfits_coreSystems["Lancelot"] = equip_shipOutfits_coreSystems["Empire Lancelot"]
equip_typeOutfits_coreSystems["Admonisher"] = {
   "Milspec Orion 4801 Core System",
}
equip_typeOutfits_coreSystems["Pacifier"] = {
   "Milspec Orion 5501 Core System",
}
equip_typeOutfits_coreSystems["Hawking"] = {
   "Milspec Orion 8601 Core System",
}
equip_typeOutfits_coreSystems["Peacemaker"] = {
   "Milspec Orion 9901 Core System",
}

equip_typeOutfits_engines["Shark"] = {
   "Tricon Zephyr Engine",
}
equip_typeOutfits_engines["Lancelot"] = equip_shipOutfits_engines["Empire Lancelot"]
equip_typeOutfits_engines["Admonisher"] = {
   "Tricon Cyclone Engine",
}
equip_typeOutfits_engines["Pacifier"] = {
   "Tricon Cyclone II Engine",
}
equip_typeOutfits_engines["Hawking"] = {
   "Tricon Typhoon Engine",
}
equip_typeOutfits_engines["Peacemaker"] = {
   "Melendez Mammoth XL Engine",
}

equip_typeOutfits_hulls["Shark"] = {
   "Nexus Light Stealth Plating", "S&K Ultralight Combat Plating",
}
equip_typeOutfits_hulls["Lancelot"] = equip_shipOutfits_hulls["Empire Lancelot"]
equip_typeOutfits_hulls["Admonisher"] = {
   "Nexus Medium Stealth Plating", "S&K Medium Combat Plating",
}
equip_typeOutfits_hulls["Pacifier"] = {
   "Nexus Medium Stealth Plating", "S&K Medium-Heavy Combat Plating",
}
equip_typeOutfits_hulls["Hawking"] = {
   "Unicorp D-48 Heavy Plating", "Unicorp D-68 Heavy Plating"
}
equip_typeOutfits_hulls["Peacemaker"] = {
   "S&K Superheavy Combat Plating",
}

equip_typeOutfits_weapons["Shark"] = {
   {
      num = 1;
      "Unicorp Banshee Launcher", "TeraCom Banshee Launcher",
      "Unicorp Mace Launcher", "TeraCom Mace Launcher",
   },
   {
      "Laser Cannon MK1", "Plasma Blaster MK1", "Ion Cannon",
   },
}
equip_typeOutfits_weapons["Lancelot"] = equip_shipOutfits_weapons["Empire Lancelot"]
equip_typeOutfits_weapons["Admonisher"] = {
   {
      num = 1;
      "TeraCom Fury Launcher", "Unicorp Headhunter Launcher",
      "Unicorp Vengeance Launcher", "Enygma Systems Spearhead Launcher"
   },
   {
      "Heavy Ripper Cannon"
   },
   {
      "Ripper Cannon"
   }
}
equip_typeOutfits_weapons["Pacifier"] = {
   {
      num = 2;
      "Heavy Laser Turret", "Railgun"
   },
   {
      num = 1;
      "TeraCom Fury Launcher", "Unicorp Headhunter Launcher",
      "Unicorp Vengeance Launcher", "Enygma Systems Spearhead Launcher",
      "Unicorp Caesar IV Launcher", "Enygma Systems Turreted Fury Launcher"
   },
   {
      num = 1;
      "Heavy Ripper Cannon", "Laser Turret MK2", "Orion Beam",
   },
   {
      "Heavy Ripper Cannon", "Laser Turret MK2"
   }
}

local emp_util_small = { { "Photo-Voltaic Nanobot Coating" } }
local emp_util_medplus = { { varied=true;
   "Hunting Combat AI", "Photo-Voltaic Nanobot Coating" } }
equip_shipOutfits_utilities["Empire Shark"]     = emp_util_small
equip_shipOutfits_utilities["Empire Lancelot"]  = emp_util_small
equip_shipOutfits_utilities["Empire Admonisher"]= emp_util_medplus
equip_shipOutfits_utilities["Empire Pacifier"]  = emp_util_medplus
equip_shipOutfits_utilities["Empire Hawking"]   = emp_util_medplus
equip_shipOutfits_utilities["Empire Peacemaker"]= emp_util_medplus

--[[
-- @brief Does empire pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   if diff.isApplied( "collective_dead" ) then
      equip_typeOutfits_weapons["Peacemaker"] = {
         {
            varied = true;
            "Empire Lancelot Fighter Bay", "Drone Fighter Bay",
         },
         {
            "Turbolaser", "Heavy Ripper Turret", "Ragnarok Beam",
         },
         {
            "Heavy Laser Turret",
         },
      }
   else
      equip_typeOutfits_weapons["Peacemaker"] = {
         {
            varied = true;
            "Empire Lancelot Fighter Bay",
         },
         {
            "Turbolaser", "Heavy Ripper Turret", "Ragnarok Beam",
         },
         {
            "Heavy Laser Turret",
         },
      }
   end

   equip_generic( p )
end
