local optimize = require 'equipopt.optimize'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'
local bioship = require 'bioship'
local prob = require "prob"

local soromid_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Soromid Brigand Bay",
   "Heavy Laser Turret", "Grave Beam",
   -- Medium Weapons
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   "Enygma Systems Spearhead Launcher", "Unicorp Caesar IV Launcher",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
   "TeraCom Imperator Launcher",
   "Plasma Cluster Cannon",
   "Plasma Turret MK2",
   -- Small Weapons
   "Plasma Cannon", "Plasma Turret MK1",
   "Plasma Blaster MK1", "Plasma Blaster MK2",
   "TeraCom Banshee Launcher", "TeraCom Mace Launcher",
   -- Point Defense
   "Spittle Tubuloid Cluster",
   -- Utility
   "Droid Repair Crew", "Milspec Scrambler",
   "Targeting Array", "Agility Combat AI",
   "Milspec Jammer", "Emergency Shield Booster",
   "Weapons Ionizer", "Sensor Array",
   "Bio-Neural Combat AI", "Nexus Stealth Coating",
   -- Heavy Structural
   "Battery III", "Shield Capacitor III", "Shield Capacitor IV",
   "Reactor Class III",
   "Large Shield Booster",
   -- Medium Structural
   "Battery II", "Shield Capacitor II", "Reactor Class II",
   "Medium Shield Booster",
   -- Small Structural
   "Improved Stabilizer", "Engine Reroute",
   "Battery I", "Shield Capacitor I", "Reactor Class I",
   "Small Shield Booster",
}}

local soromid_params = {
   ["Soromid Arx"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,2) },
         },
      } end,
}
--local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

local soromid_params_overwrite = {
   -- Prefer to use the Soromid utilities
   prefer = {
      ["Bio-Neural Combat AI"]  = 100,
      ["Nexus Stealth Coating"] = 100,
   },
   max_same_stru = 3,
}

--[[
-- @brief Does Soromid pilot equipping
--
--    @param p Pilot to equip
--]]
local function equip_soromid( p, opt_params  )
   opt_params = opt_params or {}
   local ps    = p:ship()
   local sname = ps:nameRaw()

   -- Choose parameters and make Soromidish
   local params = eparams.choose( p, soromid_params_overwrite )
   params.max_mass = 0.95 + 0.1*rnd.rnd()
   -- Per ship tweaks
   local sp = soromid_params[ sname ]
   if sp then
      params = tmerge_r( params, sp() )
   end
   params = tmerge( params, opt_params )

   -- Outfits
   local outfits = soromid_outfits
   if opt_params.outfits_add then
      outfits = eoutfits.merge{ outfits, opt_params.outfits_add }
   end

   -- Set cores
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
         cores = ecores.get( p, { all="elite" } )
      end
   end

   -- Set some meta-data
   local mem = p:memory()
   mem.equip = { type="soromid", level="elite" }

   -- Try to equip
   return optimize.optimize( p, cores, outfits, params )
end

return equip_soromid
