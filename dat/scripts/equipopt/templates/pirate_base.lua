-- PIRATE BASE TEMPLATE, DO NOT USE DIRECTLY
local optimize = require 'equipopt.optimize'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'
local bioship = require 'bioship'
local prob = require "prob"

local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

local pirate_base = {}

pirate_base.outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Pirate Hyena Bay",
   "Heavy Ripper Turret", "Railgun Turret", "Ragnarok Beam",
   "Railgun", "Heavy Laser Turret", "Grave Beam", "Heavy Ion Turret",
   -- Medium Weapons
   "Pirate Hyena Dock",
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   "Laser Turret MK2", "Turreted Vulcan Gun",
   "Plasma Turret MK2", "Orion Beam", "Grave Lance", "EMP Grenade Launcher",
   "Enygma Systems Spearhead Launcher", "Unicorp Caesar IV Launcher",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
   "TeraCom Imperator Launcher", "Heavy Ion Cannon",
   -- Small Weapons
   "Laser Cannon MK1", "Laser Cannon MK2", "Gauss Gun", "Vulcan Gun", "Plasma Blaster MK1",
   "Laser Turret MK1", "Turreted Gauss Gun", "Plasma Blaster MK2",
   "Plasma Turret MK1", "Particle Beam", "Particle Lance", "Ion Cannon",
   "TeraCom Mace Launcher", "TeraCom Banshee Launcher",
   -- Utility
   "Unicorp Scrambler", "Unicorp Light Afterburner", "Boarding Androids MK2",
   "Sensor Array", "Emergency Shield Booster", "Nexus Concealment Coating", "Stealth Burster",
   "Unicorp Medium Afterburner", "Droid Repair Crew", "Boarding Androids MK1", "Flicker Drive",
   "Scanning Combat AI", "Agility Combat AI", "Low Radiation Sensor Modulator",
   -- Heavy Structural
   "Battery III", "Shield Capacitor III", "Reactor Class III",
   "Large Shield Booster", "Nanobond Plating",
   -- Medium Structural
   "Battery II", "Shield Capacitor II", "Reactor Class II",
   "Medium Shield Booster", "Microbond Plating",
   -- Small Structural
   "Improved Stabilizer", "Engine Reroute",
   "Battery I", "Shield Capacitor I", "Reactor Class I",
   "Small Shield Booster", "Plasteel Plating",
}}

pirate_base.class = { "standard", "elite" }

pirate_base.params = {
   ["Pirate Kestrel"] = function () return {
         type_range = {
            ["Launcher"] = { max = 2 },
         },
      } end,
}
pirate_base.cores = {
   ["Pirate Revenant"] = function ()
         return {
            hull = choose_one{ "Nexus Ghost Weave", "S&K Battle Plating" },
         }
      end,
   ["Pirate Kestrel"] = function ()
          return {
             engines = "Krain Remige Engine",
             systems = "Unicorp PT-440 Core System",
             hull = "Unicorp D-58 Heavy Plating" }
      end,
   ["Pirate Starbridge"] = function (p)
         local c = ecores.get( p, { systems=pirate_base.class, hulls=pirate_base.class } )
         c.engines = choose_one{ "Nexus Arrow 700 Engine", "Krain Patagium Twin Engine", }
         c.engines_secondary = c.engines
         return c
      end,
}

pirate_base.params_overwrite = {
   -- Prefer to use the Pirate utilities
   prefer = {
      ["Scanning Combat AI"] = 100,
   },
   weap = 2, -- Focus on weapons!
   -- much more diversity for pirates
   max_same_stru = 1,
   max_same_util = 1,
}

function pirate_base.make_equip( pbase )
   --[[
   -- @brief Does Pirate pilot equipping
   --
   --    @param p Pilot to equip
   --]]
   return function ( p, opt_params )
      opt_params = opt_params or {}
      local ps = p:ship()
      local sname = ps:nameRaw()

      -- Choose parameters and make Pirateish
      local params = eparams.choose( p, pbase.params_overwrite )
      params.rnd = params.rnd * 1.5
      if ps:size() < 3 then
         params.max_same_weap = 1
      else
         params.max_same_weap = 2
      end
      params.max_mass = 0.95 + 0.2*rnd.rnd()
      -- Per ship tweaks
      local sp = pbase.params[ sname ]
      if sp then
         params = tmerge_r( params, sp() )
      end
      params = tmerge_r( params, opt_params )

      -- Outfits
      local outfits = pbase.outfits
      if opt_params.outfits_add then
         outfits = eoutfits.merge{ outfits, opt_params.outfits_add }
      end

      -- See cores
      local cores = opt_params.cores
      if not cores then
         local pircor = pbase.cores[ sname ]
         if pircor then
            cores = pircor( p )
         end
         if ps:tags().bioship then
            local stage = params.bioship_stage
            if not stage then
               local maxstage = bioship.maxstage( p )
               stage = math.max( 1, maxstage - prob.poisson_sample( 1 ) )
            end
            bioship.simulate( p, stage, params.bioship_skills )
         elseif not cores then
            cores = ecores.get( p, { all=pbase.class } )
         end
      end

      local mem = p:memory()
      mem.equip = { type="pirate", level="standard" }

      -- Try to equip
      return optimize.optimize( p, cores, outfits, params )
   end
end

return pirate_base
