local outfits = {}

--[[
-- @brief Merges two tables while removing duplicates.
--]]
function outfits.merge( t )
   local e = {}
   for i,m in ipairs(t) do
      for j,n in ipairs(m) do
         e[n] = true
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
   "Heavy Laser Turret", "Railgun Turret", "Heavy Ion Turret",
   -- Medium Weapons
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   "Unicorp Fury Launcher", "Unicorp Headhunter Launcher",
   "Unicorp Medusa Launcher", "Unicorp Vengeance Launcher",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
   "Laser Cannon MK2", "Vulcan Gun", "Plasma Blaster MK2",
   -- Small Weapons
   "Slicer", "Shredder", "Plasma Cannon",
   "Laser Turret MK1", "Turreted Gauss Gun", "Plasma Turret MK1",
   "Laser Cannon MK1", "Gauss Gun", "Plasma Blaster MK1",
   "Ion Cannon",
}
outfits.standard.utility = {
   "Unicorp Scrambler", "Unicorp Light Afterburner",
   "Sensor Array", "Unicorp Medium Afterburner", "Droid Repair Crew",
}
outfits.standard.structural = {
   -- Heavy Structural
   "Battery III", "Shield Capacitor III", "Shield Capacitor IV",
   "Reactor Class III",
   -- Medium Structural
   "Battery II", "Shield Capacitor II", "Reactor Class II",
   -- Small Structural
   "Improved Stabilizer", "Engine Reroute",
   "Battery I", "Shield Capacitor I", "Reactor Class I",
}
outfits.standard.set = outfits.merge{
   outfits.standard.weapons,
   outfits.standard.utility,
   outfits.standard.structural,
}

return outfits
