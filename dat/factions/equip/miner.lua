--[=[
require "factions/equip/generic"


-- Probability of cargo by class.
equip_classCargo["Yacht"] = .25
equip_classCargo["Luxury Yacht"] = .25
equip_classCargo["Scout"] = .25
equip_classCargo["Courier"] = .25
equip_classCargo["Freighter"] = .25
equip_classCargo["Armoured Transport"] = .25
equip_classCargo["Fighter"] = .25
equip_classCargo["Bomber"] = .25
equip_classCargo["Corvette"] = .25
equip_classCargo["Destroyer"] = .25
equip_classCargo["Cruiser"] = .25
equip_classCargo["Carrier"] = .25
equip_classCargo["Drone"] = .1
equip_classCargo["Heavy Drone"] = .1

equip_classOutfits_weapons["Yacht"] = {
   {
      "Laser Cannon MK1", "Gauss Gun", "Plasma Blaster MK1",
   },
}

equip_classOutfits_weapons["Courier"] = {
   {
      "Laser Turret MK1", "Turreted Gauss Gun", "Plasma Turret MK1",
   },
}

equip_classOutfits_weapons["Freighter"] = {
   {
      num = 1;
      "Enygma Systems Turreted Fury Launcher"
   },
   {
      "Laser Turret MK2", "Turreted Vulcan Gun", "Plasma Turret MK2",
      "Orion Beam",
   }
}


--[[
-- @brief Does miner pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   equip_generic( p )
end
--]=]
local equipopt = require 'equipopt'
local mt = require 'merge_tables'
local ecores = require 'factions.equip.cores'

local miner_outfits = {
   -- Heavy Weapons
   -- Medium Weapons
   "Enygma Systems Turreted Fury Launcher",
   "Laser Turret MK2", "Turreted Vulcan Gun", "Plasma Turret MK2",
   "Orion Beam",
   -- Small Weapons
   "Laser Turret MK1", "Turreted Gauss Gun", "Plasma Turret MK1",
   "Laser Cannon MK1", "Gauss Gun", "Plasma Blaster MK1",
   -- Utility
   "Unicorp Scrambler", "Unicorp Light Afterburner",
   "Sensor Array", "Unicorp Medium Afterburner", "Droid Repair Crew",
   -- Heavy Structural
   "Battery III", "Shield Capacitor III", "Shield Capacitor IV",
   "Reactor Class III",
   -- Medium Structural
   "Battery II", "Shield Capacitor II", "Reactor Class II",
   -- Small Structural
   "Improved Stabilizer", "Engine Reroute",
   "Battery I", "Shield Capacitor I", "Reactor Class I",
}

--[[
-- @brief Does Pirate pilot equipping
--
--    @param p Pilot to equip
--]]
function equip( p )
   -- Choose parameters and make Pirateish
   local params = equipopt.params.choose( p )

   -- See cores
   local cores = ecores.get( p, { all="normal" } )

   -- Try to equip
   equipopt.equip( p, cores, miner_outfits, params )
end
