require "factions/equip/generic"


-- Probability of cargo by class.
equip_classCargo = {
   ["Yacht"] = .5,
   ["Luxury Yacht"] = .5,
   ["Scout"] = .5,
   ["Courier"] = .5,
   ["Freighter"] = .5,
   ["Armoured Transport"] = .5,
   ["Fighter"] = .5,
   ["Bomber"] = .5,
   ["Corvette"] = .5,
   ["Destroyer"] = .5,
   ["Cruiser"] = .5,
   ["Carrier"] = .5,
   ["Drone"] = .5,
   ["Heavy Drone"] = .5
}

equip_typeOutfits_coreSystems["Hyena"] = {
   "Unicorp PT-18 Core System", "Milspec Prometheus 2203 Core System"
}
equip_typeOutfits_coreSystems["Shark"] = equip_typeOutfits_coreSystems["Hyena"]
equip_typeOutfits_coreSystems["Vendetta"] = {
   "Unicorp PT-80 Core System", "Milspec Prometheus 3603 Core System"
}
equip_typeOutfits_coreSystems["Lancelot"] = equip_typeOutfits_coreSystems["Vendetta"]
equip_typeOutfits_coreSystems["Ancestor"] = equip_typeOutfits_coreSystems["Vendetta"]
equip_typeOutfits_coreSystems["Phalanx"] = {
   "Unicorp PT-280 Core System", "Milspec Prometheus 4703 Core System"
}
equip_typeOutfits_coreSystems["Admonisher"] = equip_typeOutfits_coreSystems["Phalanx"]
equip_typeOutfits_coreSystems["Pacifier"] = {
   "Unicorp PT-400 Core System", "Milspec Prometheus 5403 Core System"
}
equip_typeOutfits_coreSystems["Kestrel"] = {
   "Unicorp PT-3400 Core System", "Milspec Prometheus 8503 Core System"
}

equip_typeOutfits_engines["Rhino"] = {
   "Unicorp Falcon 1200 Engine", "Tricon Cyclone II Engine",
}

equip_typeOutfits_weapons["Hyena"] = {
   {
      varied = true;
      "Ion Cannon", "Laser Cannon MK1", "Plasma Blaster MK1",
      "Gauss Gun", "Razor MK1",
   }
}
equip_typeOutfits_weapons["Shark"] = {
   {
      varied = true;
      "Ion Cannon", "Laser Cannon MK1", "Plasma Blaster MK2", "Vulcan Gun",
      "Shredder", "Razor MK2", "Unicorp Mace Launcher",
      "Unicorp Banshee Launcher"
   }
}
equip_typeOutfits_weapons["Vendetta"] = equip_typeOutfits_weapons["Shark"]
equip_typeOutfits_weapons["Lancelot"] = {
   {
      varied = true;
      "Ion Cannon", "Laser Cannon MK2", "Plasma Blaster MK2", "Vulcan Gun",
      "Razor MK2", "Unicorp Mace Launcher",
      "Unicorp Banshee Launcher", "Unicorp Medusa Launcher",
      "TeraCom Medusa Launcher", "Heavy Ion Cannon", "Unicorp Fury Launcher",
      "Unicorp Headhunter Launcher"
   }
}
equip_typeOutfits_weapons["Ancestor"] = {
   {
      num = 3, varied = true;
      "Unicorp Mace Launcher", "Unicorp Medusa Launcher",
      "TeraCom Medusa Launcher", "Heavy Ion Cannon", "Unicorp Fury Launcher",
      "Unicorp Headhunter Launcher", "Unicorp Banshee Launcher"
   },
   {
      "Vulcan Gun", "Shredder", "Ripper Cannon", "Plasma Blaster MK2",
      "Ion Cannon", "Unicorp Mace Launcher"
   }
}
equip_typeOutfits_weapons["Rhino"] = {
   {
      varied = true;
      "Heavy Ripper Turret", "Laser Turret MK2",
      "EMP Grenade Launcher", "Enygma Systems Turreted Fury Launcher",
      "Heavy Ion Turret", "Razor Turret MK2"
   }
}
equip_typeOutfits_weapons["Phalanx"] = {
   {
      varied = true;
      "TeraCom Fury Launcher", "Unicorp Headhunter Launcher",
      "TeraCom Medusa Launcher", "Unicorp Medusa Launcher", "Heavy Ion Cannon",
      "Mass Driver", "Plasma Turret MK2",
      "Plasma Blaster MK2", "Laser Turret MK1", "Shredder", "Ion Cannon",
      "Heavy Ripper Cannon", "Ripper Cannon"
   }
}
equip_typeOutfits_weapons["Admonisher"] = equip_typeOutfits_weapons["Phalanx"]
equip_typeOutfits_weapons["Pacifier"] = equip_typeOutfits_weapons["Phalanx"]
equip_typeOutfits_weapons["Kestrel"] = {
   {
      varied = true;
      "Repeating Railgun", "Railgun", "Heavy Laser Turret", "Railgun Turret",
      "Laser Turret MK2", "Heavy Ion Turret", "EMP Grenade Launcher",
      "Mass Driver", "Heavy Ion Cannon",
      "Turreted Vulcan Gun", "Orion Beam", "Unicorp Caesar IV Launcher",
      "TeraCom Medusa Launcher", "Razor Turret MK2",
   }
}


--[[
-- @brief Does pirate pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
