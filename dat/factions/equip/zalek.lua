require("dat/factions/equip/generic.lua")


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
   "Unicorp B-16 Heavy Plating"
}
equip_typeOutfits_hulls["Diablo"] = {
   "Unicorp B-20 Heavy Plating"
}
equip_typeOutfits_hulls["Hephaestus"] = {
   "Unicorp B-20 Heavy Plating", "S&K Superheavy Combat Plating"
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
-- @brief Does zalek pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end


-- CANNONS
function equip_forwardZlkLow ()
   return { "Particle Lance" }
end
function equip_forwardZlkMedLow ()
   return { "Orion Lance" }
end
function equip_forwardZlkMed ()
   return { "Orion Lance", "Grave Lance" }
end
-- TURRETS
function equip_turretZlkLow ()
   return { "Particle Beam" }
end
function equip_turretZlkMed ()
   return { "Pulse Beam", "Orion Beam" }
end
function equip_turretZlkHig ()
   return { "Grave Beam", "Ragnarok Beam" }
end
-- RANGED
function equip_rangedZlk ()
   return { "Electron Burst Cannon" }
end
function equip_secondaryZlk ()
   return { "Shattershield Lance", "Za'lek Hunter Launcher" }
end
-- NON-COMBAT
--[[
-- Utility slots
--]]
function equip_mediumZlkLow ()
   return { "Reactor Class I", "Unicorp Scrambler", "Small Shield Booster" }
end
function equip_mediumZlkMed ()
   return { "Reactor Class II", "Milspec Scrambler", "Medium Shield Booster" }
end
function equip_mediumZlkHig ()
   return { "Reactor Class III", "Milspec Scrambler", "Large Shield Booster" }
end

--[[
-- Structure slots
--]]
function equip_lowZlkLow ()
   return { "Battery", "Shield Capacitor", "Engine Reroute" }
end
function equip_lowZlkMed ()
   return { "Shield Capacitor II", "Shield Capacitor III", "Engine Reroute", "Battery II" }
end
function equip_lowZlkHig ()
   return { "Shield Capacitor III", "Shield Capacitor IV", "Battery III" }
end



--[[
-- @brief Equips a zalek military type ship.
--]]
function equip_empireMilitary( p, shipsize )
   local medium, low
   local use_primary, use_secondary, use_medium, use_low
   local use_forward, use_turrets, use_medturrets
   local nhigh, nmedium, nlow = p:ship():slots()
   local scramble

   -- Defaults
   medium      = { "Unicorp Scrambler" }
   weapons     = {}
   scramble    = false

   -- Equip by size and type
   if shipsize == "small" then
      local class = p:ship():class()
      cores = {
         {"Tricon Zephyr Engine", "Milspec Orion 2301 Core System", "S&K Ultralight Combat Plating"},
         {"Tricon Zephyr II Engine", "Milspec Orion 3701 Core System", "S&K Light Combat Plating"}
      }
      equip_cores(p, equip_getCores(p, shipsize, cores))

      -- Scout
      if class == "Scout" then
         equip_cores(p, "Tricon Zephyr Engine", "Milspec Orion 2301 Core System", "S&K Ultralight Stealth Plating")
         use_primary    = rnd.rnd(1,nhigh)
         addWeapons( equip_forwardLow(), use_primary )
         medium         = { "Generic Afterburner", "Milspec Scrambler" }
         use_medium     = 2
         low            = { "Solar Panel" }

      -- Drone
      elseif class == "Drone" then
        -- equip_cores(p, "Tricon Zephyr Engine", "Milspec Orion 2301 Core System", "S&K Light Stealth Plating")
         use_primary    = nhigh
         addWeapons( equip_forwardZlkLow(), use_primary )
         medium         = equip_mediumZlkLow()
         low            = equip_lowZlkLow()
		 
      -- Heavy Drone
      elseif class == "Heavy Drone" then
         use_primary    = nhigh
         addWeapons( equip_forwardZlkLow(), 2 )
         addWeapons( equip_forwardZlkMedLow(), 1 )
         addWeapons( equip_rangedZlk(), 1 )
         medium         = equip_mediumZlkLow()
         low            = equip_lowZlkLow()


      -- Bomber
      elseif class == "Bomber" then
         use_primary    = rnd.rnd(1,2)
         use_secondary  = nhigh - use_primary
         addWeapons( equip_forwardZlkLow(), use_primary )
         addWeapons( equip_rangedZlk(), use_secondary )
         medium         = equip_mediumZlkLow()
         low            = equip_lowZlkLow()

      end

   elseif shipsize == "medium" then
      local class = p:ship():class()
      cores = {
         {"Tricon Cyclone Engine", "Milspec Orion 4801 Core System", "S&K Medium Combat Plating"},
         {"Tricon Cyclone II Engine", "Milspec Orion 5501 Core System", "S&K Medium-Heavy Combat Plating"}
      }
      equip_cores(p, equip_getCores(p, shipsize, cores))

      -- Corvette
      if class == "Corvette" then
         use_secondary  = rnd.rnd(1,2)
         use_primary    = nhigh - use_secondary
         addWeapons( equip_forwardZlkMed(), use_primary )
         addWeapons( equip_secondaryZlk(), use_secondary )
         medium         = equip_mediumZlkMed()
         low            = equip_lowZlkMed()

      end

      -- Destroyer
      if class == "Destroyer" then
         use_secondary  = rnd.rnd(1,2)
         use_turrets    = nhigh - use_secondary - rnd.rnd(1,2)
         use_forward    = nhigh - use_secondary - use_turrets
         addWeapons( equip_secondaryZlk(), use_secondary )
         addWeapons( equip_turretZlkMed(), use_turrets )
         addWeapons( equip_forwardZlkMed(), use_forward )
         medium         = equip_mediumZlkMed()
         low            = equip_lowZlkMed()

      end

   else -- "large"
      -- TODO: Divide into carrier and cruiser classes.
      cores = {
         {"Tricon Typhoon Engine", "Milspec Orion 9901 Core System", "S&K Heavy Combat Plating"},
         {"Tricon Typhoon II Engine", "Milspec Orion 9901 Core System", "S&K Superheavy Combat Plating"}
      }
      equip_cores(p, equip_getCores(p, shipsize, cores))

      use_secondary  = 2
      if rnd.rnd() > 0.4 then -- Anti-fighter variant.
         use_turrets    = nhigh - use_secondary - rnd.rnd(2,3)
         use_medturrets = nhigh - use_secondary - use_turrets
         addWeapons( equip_turretZlkMed(), use_medturrets )
      else -- Anti-capital variant.
         use_turrets    = nhigh - use_secondary
      end
      addWeapons( equip_turretZlkHig(), use_turrets )
      addWeapons( equip_secondaryZlk(), use_secondary )
      medium         = equip_mediumZlkHig()
      low            = equip_lowZlkHig()

   end

   equip_ship( p, scramble, weapons, medium, low,
               use_medium, use_low )
end
