local outfits = {}

--[[
-- @brief Merges two tables while removing duplicates.
--]]
function outfits.merge( t )
   local e = {}
   for i,m in ipairs(t) do
      for j,n in ipairs(m) do
         e[ outfit.get(n) ] = true
      end
   end
   local o = {}
   for k,v in pairs(e) do
      table.insert( o, k )
   end
   return o
end

outfits.standard = {}
outfits.standard.weapons = {
   -- Heavy Weapons
   "Lancelot Bay", "Hyena Bay",
   "Heavy Laser Turret", "Railgun Turret", "Heavy Ion Turret",
   "Railgun", "Grave Beam",
   -- Medium Weapons
   "Hyena Dock",
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
   "Laser Cannon MK2", "Vulcan Gun", "Plasma Blaster MK2",
   "Laser Turret MK2", "Razor Turret MK2", "Turreted Vulcan Gun",
   "Plasma Turret MK2", "Orion Beam",
   "EMP Grenade Launcher",
   -- Small Weapons
   "Slicer", "Shredder", "Plasma Cannon",
   "Laser Turret MK1", "Turreted Gauss Gun", "Plasma Turret MK1",
   "Laser Cannon MK1", "Gauss Gun", "Plasma Blaster MK1",
   "Ion Cannon",
   -- Point Defense
   "Guardian Overseer System",
   "Guardian Interception System",
}
outfits.standard.utility = {
   --"Unicorp Scrambler", "Unicorp Light Afterburner",
   --"Sensor Array", "Unicorp Medium Afterburner", "Droid Repair Crew",
}
outfits.standard.structural = {
   -- Heavy Structural
   "Battery III", "Shield Capacitor III", "Reactor Class III",
   "Large Shield Booster",
   -- Medium Structural
   "Battery II", "Shield Capacitor II", "Reactor Class II",
   "Medium Shield Booster",
   -- Small Structural
   --"Improved Stabilizer", "Engine Reroute",
   "Battery I", "Shield Capacitor I", "Reactor Class I",
   "Small Shield Booster",
}
outfits.standard.set = outfits.merge{
   outfits.standard.weapons,
   outfits.standard.utility,
   outfits.standard.structural,
}

-- TODO proper elite outfits
outfits.elite = {}
outfits.elite.weapons = outfits.standard.weapons
outfits.elite.utility = outfits.merge{ {
   "Unicorp Scrambler", "Unicorp Light Afterburner",
   "Sensor Array", "Unicorp Medium Afterburner",
   "Droid Repair Crew",
   }, outfits.standard.utility,
}
outfits.elite.structural = outfits.merge{ {
   "Improved Stabilizer", "Engine Reroute",
   }, outfits.standard.structural
}
outfits.elite.set = outfits.merge{
   outfits.elite.weapons,
   outfits.elite.utility,
   outfits.elite.structural,
}

return outfits
