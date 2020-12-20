require "factions/equip/generic"


equip_typeOutfits_coreSystems["Sting"] = {
   "Milspec Orion 4801 Core System"
}
equip_typeOutfits_coreSystems["Demon"] = {
   "Milspec Orion 5501 Core System"
}
equip_typeOutfits_coreSystems["Mephisto"] = {
   "Milspec Orion 9901 Core System"
}
equip_typeOutfits_coreSystems["Diablo"] = {
   "Milspec Orion 9901 Core System"
}
equip_typeOutfits_coreSystems["Hephaestus"] = {
   "Milspec Orion 9901 Core System"
}

equip_typeOutfits_engines["Sting"] = {
   "Tricon Cyclone Engine"
}
equip_typeOutfits_engines["Demon"] = {
   "Tricon Cyclone II Engine"
}
equip_typeOutfits_engines["Mephisto"] = {
   "Nexus Bolt 6500 Engine", "Tricon Typhoon II Engine"
}
equip_typeOutfits_engines["Diablo"] = {
   "Melendez Mammoth XL Engine"
}
equip_typeOutfits_engines["Hephaestus"] = {
   "Melendez Mammoth XL Engine"
}

equip_typeOutfits_hulls["Sting"] = {
   "S&K Medium Stealth Plating"
}
equip_typeOutfits_hulls["Demon"] = {
   "S&K Medium Stealth Plating"
}
equip_typeOutfits_hulls["Mephisto"] = {
   "Unicorp B-48 Heavy Plating"
}
equip_typeOutfits_hulls["Diablo"] = {
   "Unicorp B-72 Heavy Plating"
}
equip_typeOutfits_hulls["Hephaestus"] = {
   "Unicorp B-72 Heavy Plating", "S&K Superheavy Combat Plating"
}

equip_typeOutfits_weapons["Sting"] = {
   {
      num = 1;
      "Grave Lance"
   },
   {
      num = 2;
      "Orion Lance"
   },
   {
      num = 1;
      "Za'lek Hunter Launcher"
   },
}
equip_typeOutfits_weapons["Demon"] = {
   {
      num = 2;
      "Grave Lance",
   },
   {
      num = 1;
      "Za'lek Hunter Launcher"
   },
   {
      "Orion Beam", "Pulse Beam",
      "Za'lek Hunter Launcher"
   }
}
equip_typeOutfits_weapons["Mephisto"] = {
   {
      num = 2, varied = true;
      "Ragnarok Beam", "Grave Beam", "Orion Beam", "Pulse Beam"
   },
   {
      num = 1;
      "Za'lek Hunter Launcher", "Za'lek Reaper Launcher"
   },
   {
      varied = true;
      "Ragnarok Beam", "Grave Beam", "Orion Beam", "Pulse Beam",
	  "Za'lek Hunter Launcher", "Za'lek Reaper Launcher"
   }
}
equip_typeOutfits_weapons["Diablo"] = equip_typeOutfits_weapons["Mephisto"] --[[ {
   {
      num = 2, varied = true;
      "Za'lek Light Drone Fighter Bay", "Za'lek Heavy Drone Fighter Bay",
      "Za'lek Bomber Drone Fighter Bay"
   },
   {
      varied = true;
      "Ragnarok Beam", "Grave Beam"
   },
   {
      varied = true;
      "Za'lek Light Drone Fighter Bay", "Za'lek Heavy Drone Fighter Bay",
      "Za'lek Bomber Drone Fighter Bay"
   }
}
--]]
equip_typeOutfits_weapons["Hephaestus"] = equip_typeOutfits_weapons["Mephisto"] --[[ {
   {
      num = 2, varied = true;
      "Za'lek Light Drone Fighter Bay", "Za'lek Heavy Drone Fighter Bay",
      "Za'lek Bomber Drone Fighter Bay"
   },
   {
      varied = true;
      "Ragnarok Beam", "Grave Beam"
   },
   {
      varied = true;
      "Za'lek Light Drone Fighter Bay", "Za'lek Heavy Drone Fighter Bay",
      "Za'lek Bomber Drone Fighter Bay"
   }
}
--]]

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

equip_typeOutfits_structurals["Demon"] = {
   {
      "Shield Capacitor IV"
   },
   {
      varied = true;
      "Battery II", "Shield Capacitor II"
   }
}
equip_typeOutfits_structurals["Diablo"] = {
   {
      varied = true;
      "Large Fuel Pod", "Battery III", "Shield Capacitor IV",
      "Shield Capacitor III"
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
