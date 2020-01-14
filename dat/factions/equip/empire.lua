include("dat/factions/equip/generic.lua")


equip_shipOutfits_coreSystems["Empire Shark"] = {
   "Milspec Orion 2301 Core System"
}
equip_shipOutfits_coreSystems["Empire Lancelot"] = {
   "Milspec Orion 3701 Core System"
}
equip_shipOutfits_coreSystems["Empire Admonisher"] = {
   "Milspec Orion 4801 Core System"
}
equip_shipOutfits_coreSystems["Empire Pacifier"] = {
   "Milspec Orion 5501 Core System"
}
equip_shipOutfits_coreSystems["Empire Hawking"] = {
   "Milspec Orion 9901 Core System"
}
equip_shipOutfits_coreSystems["Empire Peacemaker"] = {
   "Milspec Hermes 9802 Core System"
}

equip_shipOutfits_engines["Empire Shark"] = {
   "Tricon Zephyr Engine"
}
equip_shipOutfits_engines["Empire Lancelot"] = {
   "Tricon Zephyr II Engine"
}
equip_shipOutfits_engines["Empire Admonisher"] = {
   "Tricon Cyclone Engine"
}
equip_shipOutfits_engines["Empire Pacifier"] = {
   "Tricon Cyclone II Engine"
}
equip_shipOutfits_engines["Empire Hawking"] = {
   "Tricon Typhoon II Engine", "Nexus Bolt 6500 Engine"
}
equip_shipOutfits_engines["Empire Peacemaker"] = {
   "Melendez Mammoth XL Engine"
}

equip_shipOutfits_hulls["Empire Shark"] = {
   "S&K Ultralight Combat Plating"
}
equip_shipOutfits_hulls["Empire Lancelot"] = {
   "S&K Light Combat Plating"
}
equip_shipOutfits_hulls["Empire Admonisher"] = {
   "S&K Medium Combat Plating"
}
equip_shipOutfits_hulls["Empire Pacifier"] = {
   "S&K Medium-Heavy Combat Plating"
}
equip_shipOutfits_hulls["Empire Hawking"] = {
   "Unicorp B-20 Heavy Plating", "S&K Superheavy Combat Plating"
}
equip_shipOutfits_hulls["Empire Peacemaker"] = {
   "S&K Superheavy Combat Plating"
}

equip_shipOutfits_weapons["Empire Shark"] = {
   {
      num = 1;
      "Unicorp Banshee Launcher", "Ripper Cannon"
   },
   {
      "Ripper Cannon", "Laser Cannon MK3"
   }
}
-- Empire Lancelot weapons specified in generic.lua
equip_shipOutfits_weapons["Empire Admonisher"] = {
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
equip_shipOutfits_weapons["Empire Pacifier"] = {
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
equip_shipOutfits_weapons["Empire Hawking"] = {
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


--[[
-- @brief Does empire pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   if diff.isApplied( "collective_dead" ) then
      equip_shipOutfits_weapons["Empire Peacemaker"] = {
         {
            num = 2, varied = true;
            "Turbolaser", "Heavy Laser", "Ragnarok", "Grave Beam"
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
      equip_shipOutfits_weapons["Empire Peacemaker"] = {
         {
            num = 2, varied = true;
            "Turbolaser", "Heavy Laser", "Ragnarok", "Grave Beam"
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

   equip_generic( p )
end
