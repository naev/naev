local optimize = require 'equipopt.optimize'
local mt = require 'merge_tables'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'

local proteron_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Proteron Derivative Fighter Bay",
   "Heavy Razor Turret", "Grave Beam", "Railgun",
   "Heavy Laser Turret", "Railgun Turret", "Ragnarok Beam",
   "Heavy Ripper Turret",
   -- Medium Weapons
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   -- Small Weapons
   "Unicorp Banshee Launcher", "TeraCom Banshee Launcher",
   "Laser Cannon MK1", "Plasma Blaster MK1",
   -- Utility
   "Droid Repair Crew", "Milspec Scrambler",
   "Targeting Array", "Agility Combat AI",
   "Milspec Jammer", "Emergency Shield Booster",
   "Weapons Ionizer", "Sensor Array",
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

local proteron_params = {
   --["Proteron Demon"] = function () return {
}
local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

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
   local params = eparams.choose( p )
   -- Prefer to use the Proteron utilities
   params.max_same_stru = 3
   params.max_mass = 0.95 + 0.1*rnd.rnd()
   -- Per ship tweaks
   local sp = proteron_params[ sname ]
   if sp then
      params = mt.merge_tables_recursive( params, sp() )
   end
   params = mt.merge_tables( params, opt_params )

   -- See cores
   cores = ecores.get( p, { all="elite" } )

   -- Try to equip
   return optimize.optimize( p, cores, proteron_outfits, params )
end


return equip_proteron
