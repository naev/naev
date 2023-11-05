local optimize = require 'equipopt.optimize'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'

local proteron_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Proteron Dalton Bay",
   "Grave Beam", "Railgun",
   "Heavy Laser Turret", "Railgun Turret", "Ragnarok Beam",
   "Heavy Ripper Turret",
   -- Medium Weapons
   "Proteron Dalton Dock",
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   "Laser Turret MK2", "Plasma Turret MK2",
   -- Small Weapons
   "Laser Turret MK1", "Laser Cannon MK1",
   "Plasma Turret MK1", "Plasma Blaster MK1",
   -- Utility
   "Droid Repair Crew", "Milspec Scrambler",
   "Targeting Array",
   "Hunting Combat AI", "Agility Combat AI",
   "Milspec Impacto-Plastic Coating", "Photo-Voltaic Nanobot Coating",
   "Milspec Jammer", "Emergency Shield Booster",
   "Weapons Ionizer", "Sensor Array",
   -- Heavy Structural
   "Battery III", "Shield Capacitor III",
   "Reactor Class III", "Large Shield Booster",
   "Biometal Armour",
   -- Medium Structural
   "Battery II", "Shield Capacitor II",
   "Reactor Class II", "Medium Shield Booster",
   -- Small Structural
   "Improved Stabilizer", "Engine Reroute", "Plasteel Plating",
   "Battery I", "Shield Capacitor I",
   "Reactor Class I", "Small Shield Booster",
}}

local proteron_params = {
   ["Proteron Dalton"] = function () return {
   type_range = {
            ["Launcher"] = { max = 1 },
         },
      } end,
   ["Proteron Hippocrates"] = function () return {
   energy = 0.8, -- neglect energy and prefer launchers and fighter bays
   launcher = 1.2,
   fighterbay = 1.1,
   type_range = {
            ["Launcher"] = { min = 1, max = rnd.rnd(1,3) },
         },
      } end,
   ["Proteron Gauss"] = function () return {
         type_range = {
            ["Launcher"] = { max = 1 },
         }
      } end,
   ["Proteron Pythagoras"] = function () return {
   type_range = {
            ["Launcher"] = { max = 1 },
            ["Bolt Turret"] = { min = 1 },
         },
      } end,
   ["Proteron Archimedes"] = function () return {
   type_range = {
            ["Launcher"] = { max = 2 },
            ["Bolt Turret"] = { min = 1 },
         },
      } end,
}
--local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

local proteron_params_overwrite = {
   turret = 1.1,
   launcher = 0.9,
   max_same_stru = 3,
}

--[[
-- @brief Does Proteron pilot equipping
--
--    @param p Pilot to equip
--]]
local function equip_proteron( p, opt_params )
   opt_params = opt_params or {}
   local ps    = p:ship()
   local sname = ps:nameRaw()

   -- Choose parameters and make Proteronish
   local params = eparams.choose( p, proteron_params_overwrite )
   params.max_mass = 0.95 + 0.1*rnd.rnd()
   -- Per ship tweaks
   local sp = proteron_params[ sname ]
   if sp then
      params = tmerge_r( params, sp() )
   end
   params = tmerge( params, opt_params )

   -- Outfits
   local outfits = proteron_outfits
   if opt_params.outfits_add then
      outfits = eoutfits.merge{ outfits, opt_params.outfits_add }
   end

   -- See cores
   local cores = opt_params.cores
   if not cores then
      cores = ecores.get( p, { all="elite" } )
   end

   -- Set some meta-data
   local mem = p:memory()
   mem.equip = { type="proteron", level="elite" }

   -- Try to equip
   return optimize.optimize( p, cores, outfits, params )
end


return equip_proteron
