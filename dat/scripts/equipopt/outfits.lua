local outfits = {}

--[[
-- @brief Merges two tables while removing duplicates.
--]]
function outfits.merge( t )
   local e = {}
   for i,m in ipairs(t) do
      for j,n in ipairs(m) do
         local o = outfit.get(n)
         e[ o:nameRaw() ] = o
      end
   end
   local o = {}
   for k,v in pairs(e) do
      table.insert( o, v )
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
   "Hyena Dock", "Heavy Ripper Cannon", "Mass Driver",
   "Enygma Systems Turreted Fury Launcher", "Heavy Ion Cannon",
   "Enygma Systems Turreted Headhunter Launcher",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "Turreted Vulcan Gun", "EMP Grenade Launcher",
   "Plasma Turret MK2", "Orion Beam", "Razor Battery S2",
   -- Small Weapons
   "Shredder", "Plasma Cannon", "Laser Cannon MK2", "Vulcan Gun", "Plasma Blaster MK2",
   "Laser Turret MK2", "Laser Turret MK1", "Turreted Gauss Gun", "Plasma Turret MK1",
   "Laser Cannon MK1", "Gauss Gun", "Plasma Blaster MK1", "Razor Artillery S1",
   "Ion Cannon", "Razor Artillery S2", "Razor Artillery S3", "Ripper Cannon", "Particle Beam",
   -- Point Defense
   "Guardian Overseer System",
   "Guardian Interception System",
}
outfits.standard.utility = {
   "Unicorp Scrambler", "Sensor Array", "Unicorp Light Afterburner",
   "Emergency Shield Booster", "Unicorp Medium Afterburner",
}
outfits.standard.structural = {
   -- Heavy Structural
   "Battery III", "Shield Capacitor III", "Reactor Class III",
   "Large Shield Booster", "Nanobond Plating",
   -- Medium Structural
   "Battery II", "Shield Capacitor II", "Reactor Class II",
   "Medium Shield Booster", "Microbond Plating",
   -- Small Structural
   --"Improved Stabilizer", "Engine Reroute",
   "Battery I", "Shield Capacitor I", "Reactor Class I",
   "Small Shield Booster", "Plasteel Plating",
}
outfits.standard.set = outfits.merge{
   outfits.standard.weapons,
   outfits.standard.utility,
   outfits.standard.structural,
}

-- TODO proper elite outfits
outfits.elite = {}
outfits.elite.weapons = outfits.merge{ {
   "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
   "TeraCom Mace Launcher", "Enygma Systems Spearhead Launcher",
   "Unicorp Caesar IV Launcher", "Enygma Systems Huntsman Launcher",
   "Plasma Cluster Cannon", "Grave Lance", "Ragnarok Beam",
   "Heavy Ripper Turret", "Orion Lance", "Particle Lance",
   }, outfits.standard.weapons,
}
outfits.elite.utility = outfits.merge{ {
   "Targeting Array", "Nexus Concealment Coating",
   "Droid Repair Crew", "Hyperbolic Blink Engine",
   "Flicker Drive", "Milspec Scrambler",
   "Milspec Impacto-Plastic Coating", "Photo-Voltaic Nanobot Coating",
   }, outfits.standard.utility,
}
outfits.elite.structural = outfits.merge{ {
   "Improved Stabilizer", "Engine Reroute", "Auxiliary Processing Unit I",
   "Auxiliary Processing Unit II", "Auxiliary Processing Unit III",
   "Auxiliary Processing Unit IV", "Battery IV", "Shield Capacitor IV",
   }, outfits.standard.structural
}
outfits.elite.set = outfits.merge{
   outfits.elite.weapons,
   outfits.elite.utility,
   outfits.elite.structural,
}

return outfits
