--[[
--
--    Outfit defines for the equipping script
--
-- This contains defines to help simplify the addition of new outfits for
--  different ships.
--]]



--[[
-- @brief Merges tables together
--]]
function table_merge( t, ... )
   args = {...}
   for ak, av in ipairs( args ) do
      for k,v in ipairs(av) do
         t[ #t+1 ] = v
      end
   end
   return t
end


--[[
-- Forward mounts
--]]
function equip_forwardLow ()
   return { "Laser Cannon MK1", "Ion Cannon", "Razor MK1", "Gauss Gun" }
end
function equip_forwardMed ()
   return { "Mass Driver MK1" }
end
function equip_forwardHig ()
   return { }
end
function equip_forwardMedLow ()
   return table_merge( equip_forwardLow(), equip_forwardMed() )
end
function equip_forwardHigMedLow ()
   return table_merge( equip_forwardLow(), equip_forwardMed(), equip_forwardHig() )
end


--[[
-- Turret mounts
--]]
function equip_turretLow ()
   return { "Razor Turret MK1", "Turreted Gauss Gun", "Laser PD MK1" }
end
function equip_turretMed ()
   return { "Laser Turret MK1", "Laser Turret MK2" }
end
function equip_turretHig ()
   return { "Heavy Laser" }
end
function equip_turretMedLow ()
   return table_merge( equip_turretLow(), equip_turretMed() )
end
function equip_turretHigMedLow ()
   return table_merge( equip_turretLow(), equip_turretMed(), equip_turretHig() )
end


--[[
-- Ranged weapons
--]]
function equip_rangedLow ()
   return { "Unicorp Fury Launcher" }
end
function equip_rangedMed ()
   return { "Unicorp Headhunter Launcher" }
end
function equip_rangedHig ()
   return { }
end
function equip_rangedMedLow ()
   return table_merge( equip_rangedLow(), equip_rangedMed() )
end
function equip_rangedHigMedLow ()
   return table_merge( equip_rangedLow(), equip_rangedMed(), equip_rangedHig() )
end


--[[
-- Secondary weapons
--]]
function equip_secondaryLow ()
   return { "Unicorp Fury Launcher", "Unicorp Mace Launcher" }
end
function equip_secondaryMed ()
   return { "Unicorp Banshee Launcher", "Unicorp Headhunter Launcher" }
end
function equip_secondaryHig ()
   return { }
end
function equip_secondaryMedLow ()
   return table_merge( equip_secondaryLow(), equip_secondaryMed() )
end
function equip_secondaryHigMedLow ()
   return table_merge( equip_secondaryLow(), equip_secondaryMed(), equip_secondaryHig() )
end


--[[
-- Medium slots
--]]
function equip_mediumLow ()
   return { "Reactor Class I", "Unicorp Scrambler" }
end
function equip_mediumMed ()
   return { "Reactor Class II", "Milspec Scrambler" }
end
function equip_mediumHig ()
   return { "Reactor Class III", "Milspec Scrambler" }
end


--[[
-- Low slots
--]]
function equip_lowLow ()
   return { "Battery", "Shield Capacitor", "Plasteel Plating", "Engine Reroute" }
end
function equip_lowMed ()
   return { "Shield Capacitor II", "Shield Capacitor III", "Plasteel Plating",
            "Engine Reroute", "Battery II" }
end
function equip_lowHig ()
   return { "Shield Capacitor III", "Shield Capacitor IV", "Battery III" }
end
