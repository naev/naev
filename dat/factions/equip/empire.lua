require "dat/factions/equip/generic.lua"


equip_typeOutfits_coreSystems["Shark"] = {
   "Milspec Orion 2301 Core System"
}
equip_typeOutfits_coreSystems["Lancelot"] = {
   "Milspec Orion 3701 Core System"
}
equip_typeOutfits_coreSystems["Admonisher"] = {
   "Milspec Orion 4801 Core System"
}
equip_typeOutfits_coreSystems["Pacifier"] = {
   "Milspec Orion 5501 Core System"
}
equip_typeOutfits_coreSystems["Hawking"] = {
   "Milspec Orion 9901 Core System"
}
equip_typeOutfits_coreSystems["Peacemaker"] = {
   "Milspec Hermes 9802 Core System"
}

equip_typeOutfits_engines["Shark"] = {
   "Tricon Zephyr Engine"
}
equip_typeOutfits_engines["Lancelot"] = {
   "Tricon Zephyr II Engine"
}
equip_typeOutfits_engines["Admonisher"] = {
   "Tricon Cyclone Engine"
}
equip_typeOutfits_engines["Pacifier"] = {
   "Tricon Cyclone II Engine"
}
equip_typeOutfits_engines["Hawking"] = {
   "Tricon Typhoon II Engine", "Nexus Bolt 6500 Engine"
}
equip_typeOutfits_engines["Peacemaker"] = {
   "Melendez Mammoth XL Engine"
}

equip_typeOutfits_hulls["Shark"] = {
   "S&K Ultralight Combat Plating"
}
equip_typeOutfits_hulls["Lancelot"] = {
   "S&K Light Combat Plating"
}
equip_typeOutfits_hulls["Admonisher"] = {
   "S&K Medium Combat Plating"
}
equip_typeOutfits_hulls["Pacifier"] = {
   "S&K Medium-Heavy Combat Plating"
}
equip_typeOutfits_hulls["Hawking"] = {
   "Unicorp B-20 Heavy Plating", "S&K Superheavy Combat Plating"
}
equip_typeOutfits_hulls["Peacemaker"] = {
   "S&K Superheavy Combat Plating"
}

equip_typeOutfits_weapons["Shark"] = {
   {
      num = 1;
      "Unicorp Banshee Launcher", "Ripper Cannon"
   },
   {
      "Ripper Cannon", "Laser Cannon MK3"
   }
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
      "Heavy Ripper Turret", "Railgun"
   },
   {
      num = 1;
      "TeraCom Fury Launcher", "Unicorp Headhunter Launcher",
      "Unicorp Vengeance Launcher", "Enygma Systems Spearhead Launcher",
      "Unicorp Caesar IV Launcher", "Enygma Systems Turreted Fury Launcher"
   },
   {
      num = 1;
      "Heavy Ripper Cannon", "Laser Turret MK3", "Orion Beam", "Pulse Beam"
   },
   {
      "Heavy Ripper Cannon", "Laser Turret MK3"
   }
}
equip_typeOutfits_weapons["Hawking"] = {
   {
      num = 2;
      "Heavy Laser"
   },
   {
      num = 2;
      "Turbolaser", "Ragnarok Beam", "Grave Beam"
   },
   {
      num = 1;
      "Turbolaser", "Ragnarok Beam", "Grave Beam"
   },
   {
      num = 1;
      "Turbolaser", "Heavy Laser"
   },
   {
      "Heavy Laser"
   }
}
equip_typeOutfits_weapons["Peacemaker"] = equip_typeOutfits_weapons["Hawking"]

--[[
-- @brief Does empire pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   --[[
   if diff.isApplied( "collective_dead" ) then
      equip_typeOutfits_weapons["Peacemaker"] = {
         {
            num = 2, varied = true;
            "Turbolaser", "Heavy Laser", "Ragnarok Beam", "Grave Beam"
         },
         {
            varied = true;
            "Empire Lancelot Fighter Bay", "Drone Fighter Bay"
         },
         {
            varied = true;
            "Heavy Ripper Turret", "Laser Turret MK3"
         }
      }
   else
      equip_typeOutfits_weapons["Peacemaker"] = {
         {
            num = 2, varied = true;
            "Turbolaser", "Heavy Laser", "Ragnarok Beam", "Grave Beam"
         },
         {
            varied = true;
            "Empire Lancelot Fighter Bay"
         },
         {
            varied = true;
            "Heavy Ripper Turret", "Laser Turret MK3"
         }
      }
   end
   --]]
   
   equip_generic( p )
end
