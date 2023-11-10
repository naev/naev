local optimize = require 'equipopt.optimize'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'
local bioship = require 'bioship'
local prob = require "prob"

local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

local pirate_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Pirate Hyena Dock", "Pirate Hyena Bay",
   "Heavy Ripper Turret", "Railgun Turret", "Ragnarok Beam",
   "Railgun", "Heavy Laser Turret", "Grave Beam", "Heavy Ion Turret",
   "Heavy Laser Turret", "Grave Beam", "Heavy Ion Turret",
   -- Medium Weapons
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   "Laser Turret MK2", "Turreted Vulcan Gun",
   "Plasma Turret MK2", "Orion Beam", "EMP Grenade Launcher",
   "Enygma Systems Spearhead Launcher", "Unicorp Caesar IV Launcher",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
   "TeraCom Imperator Launcher",
   "Laser Cannon MK2", "Vulcan Gun", "Plasma Blaster MK2",
   "Orion Lance", "Ion Cannon",
   -- Small Weapons
   "Laser Cannon MK1", "Gauss Gun", "Plasma Blaster MK1",
   "Laser Turret MK1", "Turreted Gauss Gun",
   "Plasma Turret MK1", "Particle Beam",
   "TeraCom Mace Launcher", "TeraCom Banshee Launcher",
   -- Utility
   "Unicorp Scrambler", "Unicorp Light Afterburner",
   "Sensor Array", "Hellburner", "Emergency Shield Booster",
   "Unicorp Medium Afterburner", "Droid Repair Crew",
   "Scanning Combat AI",
   -- Heavy Structural
   "Battery III", "Shield Capacitor III", "Reactor Class III",
   "Large Shield Booster",
   -- Medium Structural
   "Battery II", "Shield Capacitor II", "Reactor Class II",
   "Medium Shield Booster",
   -- Small Structural
   "Improved Stabilizer", "Engine Reroute",
   "Battery I", "Shield Capacitor I", "Reactor Class I",
   "Small Shield Booster",
}}

local pirate_class = { "standard", "elite" }

local pirate_params = {
   ["Pirate Kestrel"] = function () return {
         type_range = {
            ["Launcher"] = { max = 2 },
         },
      } end,
}
local pirate_cores = {
   ["Pirate Kestrel"] = function (_p)
         return { "Krain Remige Engine", "Unicorp PT-500 Core System", "Unicorp D-58 Heavy Plating" }
      end,
   ["Pirate Starbridge"] = function (p)
         local c = ecores.get( p, { systems=pirate_class, hulls=pirate_class } )
         table.insert( c, choose_one{ "Unicorp Falcon 1400 Engine", "Krain Patagium Engine", } )
         return c
      end,
}

local pirate_params_overwrite = {
   -- Prefer to use the Pirate utilities
   prefer = {
      ["Scanning Combat AI"] = 100,
   },
   weap = 2, -- Focus on weapons!
   -- much more diversity for pirates
   max_same_stru = 1,
   max_same_util = 1,
}

--[[
-- @brief Does Pirate pilot equipping
--
--    @param p Pilot to equip
--]]
local function equip_pirate( p, opt_params )
   opt_params = opt_params or {}
   local ps = p:ship()
   local sname = ps:nameRaw()

   -- Choose parameters and make Pirateish
   local params = eparams.choose( p, pirate_params_overwrite )
   params.rnd = params.rnd * 1.5
   if ps:size() < 3 then
      params.max_same_weap = 1
   else
      params.max_same_weap = 2
   end
   params.max_mass = 0.95 + 0.2*rnd.rnd()
   -- Per ship tweaks
   local sp = pirate_params[ sname ]
   if sp then
      params = tmerge_r( params, sp() )
   end
   params = tmerge( params, opt_params )

   -- Outfits
   local outfits = pirate_outfits
   if opt_params.outfits_add then
      outfits = eoutfits.merge{ outfits, opt_params.outfits_add }
   end

   -- See cores
   local cores = opt_params.cores
   if not cores then
      if ps:tags().bioship then
         local stage = params.bioship_stage
         if not stage then
            local maxstage = bioship.maxstage( p )
            stage = math.max( 1, maxstage - prob.poisson_sample( 1 ) )
         end
         bioship.simulate( p, stage, params.bioship_skills )
      else
      local pircor = pirate_cores[ sname ]
         if pircor then
            cores = pircor( p )
         else
            cores = ecores.get( p, { all=pirate_class } )
         end
      end
   end

   local mem = p:memory()
   mem.equip = { type="pirate", level="standard" }

   -- Try to equip
   return optimize.optimize( p, cores, outfits, params )
end

return equip_pirate
