local optimize = require 'equipopt.optimize'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'

local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

local escort_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Lancelot Bay",
   "Hyena Dock",
   "Hyena Bay",
   "Turbolaser", "Heavy Ripper Turret", "Railgun Turret", "Ragnarok Beam",
   "Railgun", "Heavy Laser Turret", "Grave Beam", "Heavy Ion Turret",
   "Heavy Laser Turret",
   -- Medium Weapons
   "Heavy Ripper Cannon",
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   "Laser Turret MK2", "Razor Turret MK2", "Turreted Vulcan Gun",
   "Plasma Turret MK2", "Orion Beam", "EMP Grenade Launcher",
   "TeraCom Headhunter Launcher",
   "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
   "Enygma Systems Spearhead Launcher", "Unicorp Caesar IV Launcher",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
   "TeraCom Imperator Launcher",
   "Laser Cannon MK2", "Razor MK2", "Vulcan Gun", "Plasma Blaster MK2",
    "Grave Lance", "Orion Lance", "Ion Cannon",
   -- Small Weapons
   "Laser Cannon MK1", "Razor MK1", "Gauss Gun", "Plasma Blaster MK1",
   "Laser Turret MK1", "Razor Turret MK1", "Turreted Gauss Gun",
   "Plasma Turret MK1", "Particle Beam", "Particle Lance",
   "TeraCom Mace Launcher", "TeraCom Banshee Launcher",
   "TeraCom Banshee Launcher",
   -- Utility
   "Unicorp Scrambler", "Unicorp Light Afterburner",
   "Sensor Array", "Hellburner", -- "Emergency Shield Booster",
   "Unicorp Medium Afterburner", "Droid Repair Crew",
   "Scanning Combat AI", "Hunting Combat AI",
   "Photo-Voltaic Nanobot Coating",
   "Adaptive Stealth Plating",
--   "Blink Drive",
   "Milspec Scrambler",
   "Targeting Array", "Agility Combat AI",
   "Milspec Jammer",
   "Weapons Ionizer",
   "Faraday Tempest Coating", "Hive Combat AI",
   -- Heavy Structural
   "Biometal Armour",
   "Battery IV", "Large Fuel Pod",
   "Battery III", "Shield Capacitor III", "Shield Capacitor IV",
   "Reactor Class III",
   "Large Shield Booster",
   -- Medium Structural
   "Battery II", "Shield Capacitor II", "Reactor Class II",
   "Active Plating", "Medium Shield Booster",
   -- Small Structural
   "Battery I", "Shield Capacitor I", "Reactor Class I",
   "Small Shield Booster",
}}

local escort_class = { "standard", "elite" }

local escort_params = {
   ["Kestrel"] = function () return {
         type_range = {
            ["Launcher"] = { max = 2 },
         },
      } end,
	["Pirate Kestrel"] = function () return {
		prefer = {
			[ "Enygma Systems Spearhead Launcher"] = 7, ["TeraCom Headhunter Launcher"] = 4,
			["TeraCom Medusa Launcher"] = 3, ["Enygma Systems Turreted Fury Launcher"] = 2
		},
         type_range = {
            ["Launcher"] = { max = 4 },
         },
      } end,
	["Mule"] = function() return {
		fighterbay = 1.2,
		disable = 1.1,
		move = 0.5,
		prefer = { ["Blink Drive"] = 30, ["Milspec Jammer"] = 40, ["Droid Repair Crew"] = 2 },
		type_range = {
			[ "Launcher" ] = { max=1 },
			[ "Beam Weapon" ] = { min=1, max=2 }
		},
	} end,
	["Goddard"] = function () return {
		fighterbay = 1.5,
		disable = 2,
		move = 1.5,
		prefer = {
			["Droid Repair Crew"] = 2, ["Biometal Armour"] = 60, ["Engine Reroute"] = 2,
			["Hyperbolic Blink Engine"] = 5, ["Enygma Systems Huntsman Launcher"] = 100,
			["Agility Combat AI"] = 100
		},
		type_range = {
			[ "Launcher" ] = { max=3 }
		},
	} end,
}

local escort_cores = {
   ["Pirate Kestrel"] = function (p)
         local c = ecores.get( p, { systems=escort_class, hulls=escort_class } )
         table.insert( c, choose_one{ "Nexus Bolt 3500 Engine", "Krain Remige Engine", "Tricon Typhoon Engine", } )
         return c
      end,
	["Kestrel"] = function () return {
         choose_one{ "Unicorp PT-2200 Core System", "Milspec Orion 8601 Core System", "Milspec Thalos 9802 Core System", "Milspec Orion 9901 Core System" },
         choose_one{ "Nexus Bolt 3500 Engine", "Krain Remige Engine", "Tricon Typhoon Engine", },
		 choose_one{ "Unicorp D-48 Heavy Plating", "S&K Heavy Combat Plating" },

      } end,
	["Goddard"] = function () return {
         choose_one{ "Milspec Thalos 9802 Core System", "Milspec Orion 9901 Core System" },
         choose_one{ "Tricon Typhoon Engine II", "Melendez Mammoth XL Engine"},
		 choose_one{ "S&K Superheavy Combat Plating", "S&K Heavy Combat Plating" },
      } end,
	["Pirate Starbridge"] = function (p)
         local c = ecores.get( p, { systems=escort_class, hulls=escort_class } )
         table.insert( c, choose_one{ "Unicorp Falcon 1300 Engine", "Krain Patagium Engine", "Tricon Cyclone Engine"} )
         return c
      end,
   ["Starbridge"] = function (p)
         local c = ecores.get( p, { systems=escort_class, hulls=escort_class } )
         table.insert( c, choose_one{ "Unicorp Falcon 1300 Engine", "Krain Patagium Engine", "Tricon Cyclone Engine"} )
         return c
      end,
   ["Shark"] = function () return {
         choose_one{ "Milspec Orion 2301 Core System", "Milspec Thalos 2202 Core System" },
         "Tricon Zephyr Engine",
         choose_one{ "Nexus Light Stealth Plating", "S&K Ultralight Combat Plating" },
      } end,
   ["Empire Shark"] = function () return {
         choose_one{ "Milspec Orion 2301 Core System", "Milspec Thalos 2202 Core System" },
         "Tricon Zephyr Engine",
         choose_one{ "Nexus Light Stealth Plating", "S&K Ultralight Combat Plating" },
      } end,
   ["Mule"] = function() return {
		 choose_one{ "Milspec Orion 5501 Core System", "Milspec Thalos 5402 Core System", "Unicorp PT-310 Core System" },
		 "Melendez Buffalo XL Engine",
		 choose_one{"S&K Medium Combat Plating", "Unicorp D-24 Medium Plating", "S&K Medium-Heavy Combat Plating", "Patchwork Medium Plating" },
   } end,
}

local escort_params_overwrite = {
   weap = 1.5, -- Focus on weapons
   disable = 2.4, -- prefer disablers
   -- some nice preferable escort outfits
  prefer = {
		[ "TeraCom Medusa Launcher"] = 6 ,
		[ "Enygma Systems Spearhead Launcher"] = 4, ["TeraCom Headhunter Launcher"] = 10, ["Large Shield Booster"] = 2,
		[ "Shield Capacitor IV"] = 2, ["Biometal Armour"] = 2
	},

   -- not too much diversity, but some
   max_same_stru = 2,
   max_same_util = 1,
   cargo = 0.0,
   constant = 4,
   rnd = 0.3,
}

--[[
-- @brief Does Escort pilot equipping
--
--    @param p Pilot to equip
--]]
local function equip_escort( p, opt_params )
   opt_params = opt_params or {}
   local ps = p:ship()
   local sname = ps:nameRaw()

   -- Choose parameters and make Pirateish
   local params = eparams.choose( p, escort_params_overwrite )
   params.rnd = params.rnd * 1.5
	if ps:size() < 3 then
      params.max_same_weap = 2
   else
      params.max_same_weap = 3
   end
   params.max_mass = 0.95 + 1.2*rnd.rnd()
   -- Per ship tweaks
   local sp = escort_params[ sname ]
   if sp then
      params = tmerge_r( params, sp() )
   end
   params = tmerge_r( params, opt_params )

   -- See cores
   local cores
   local esccor = escort_cores[ sname ]
   if esccor then
      cores = esccor( p )
   else
      cores = ecores.get( p, { all=escort_class } )
   end

   local mem = p:memory()
   mem.equip = { type="escort", level="standard" }

   -- Try to equip
   return optimize.optimize( p, cores, escort_outfits, params )
end

return equip_escort
