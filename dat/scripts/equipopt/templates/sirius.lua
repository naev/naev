local optimize = require 'equipopt.optimize'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'
local eparams = require 'equipopt.params'

local sirius_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Sirius Fidelity Bay", "Sirius Shaman Bay",
   "Heavy Ion Turret",
   "Ragnarok Beam", "Grave Beam",
   "Disruptor Battery S2",
   "Disruptor Artillery S2",
   -- Medium Weapons
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   "Enygma Systems Spearhead Launcher", "Unicorp Caesar IV Launcher",
   "TeraCom Fury Launcher", "TeraCom Headhunter Launcher",
   "TeraCom Medusa Launcher", "TeraCom Vengeance Launcher",
   "TeraCom Imperator Launcher",
   "Disruptor Artillery S2", "Razor Battery S2",
   -- Small Weapons
   "Razor Artillery S3", "Razor Artillery S2", "Razor Artillery S1",
   "Ion Cannon",
   -- Utility
   "Droid Repair Crew", "Milspec Scrambler",
   "Targeting Array", "Agility Combat AI",
   "Milspec Jammer", "Emergency Shield Booster",
   "Weapons Ionizer", "Sensor Array",
   "Pinpoint Combat AI", "Lattice Thermal Coating",
   -- Heavy Structural
   "Battery III", "Shield Capacitor III",
   "Reactor Class III", "Large Shield Booster",
   -- Medium Structural
   "Battery II", "Shield Capacitor II",
   "Reactor Class II", "Medium Shield Booster",
   -- Small Structural
   "Improved Stabilizer", "Engine Reroute",
   "Battery I", "Shield Capacitor I",
   "Reactor Class I", "Small Shield Booster",
   -- Flow stuff
   "Large Flow Amplifier", "Medium Flow Amplifier", "Small Flow Amplifier",
   "Large Flow Resonator", "Medium Flow Resonator", "Small Flow Resonator",
   --"Large Meditation Chamber", "Medium Meditation Chamber", "Small Meditation Chamber",
}}

local sirius_abilities = {
   outfit.get("Seeking Chakra"),
   outfit.get("Feather Drive"),
   outfit.get("Cleansing Flames"),
   outfit.get("Astral Projection"),
   outfit.get("Avatar of Sirichana"),
}
local sirius_abilities_w = {
   10,
   6,
   2,
   2,
   1,
}
local nw = 0
for k,v in ipairs(sirius_abilities_w) do
   nw = nw+v
   sirius_abilities_w[k] = nw+v
end
for k,v in ipairs(sirius_abilities_w) do
   sirius_abilities_w[k] = sirius_abilities_w[k] / nw
end

local sirius_params = {
   --["Sirius Demon"] = function () return {
}
--local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end
local sirius_cores = {
   ["Sirius Fidelity"] = function () return {
         "Milspec Orion 2301 Core System",
         "Tricon Zephyr Engine",
         "S&K Ultralight Combat Plating",
      } end,
}

local sirius_params_overwrite = {
   -- Prefer to use the Sirius utilities
   prefer = {
      ["Pinpoint Combat AI"]      = 100,
      ["Lattice Thermal Coating"] = 100,
   },
   max_same_stru = 3,
}

--[[
-- @brief Does Sirius pilot equipping
--
-- Some useful parameters for opt_params:
--  * noflow=true: pilot will not spawn with flow abilities
--  * flow_ability=X: pilot will spawn with flow ability X (and outfits necessary to use it)
--
--    @param p Pilot to equip
--]]
local function equip_sirius( p, opt_params )
   opt_params = opt_params or {}
   local ps    = p:ship()
   local sname = ps:nameRaw()

   -- Choose parameters and make Siriusish
   local params = eparams.choose( p, sirius_params_overwrite )
   params.max_mass = 0.95 + 0.1*rnd.rnd()
   -- Per ship tweaks
   local sp = sirius_params[ sname ]
   if sp then
      params = tmerge_r( params, sp() )
   end
   params = tmerge_r( params, opt_params )

   -- Outfits
   local outfits = sirius_outfits
   if opt_params.outfits_add then
      outfits = eoutfits.merge{ outfits, opt_params.outfits_add }
   end

   -- See cores
   local cores = opt_params.cores
   if not cores then
      local srscor = sirius_cores[ sname ]
      if srscor then
         cores = srscor()
      else
         cores = ecores.get( p, { all="elite" } )
      end
   end

   -- Try to give a flow ability if not from a fighter bay randomly
   local issirius = ps:tags().sirius
   if (not opt_params.noflow and (issirius and rnd.rnd() < 0.8) or (not issirius and rnd.rnd() < 0.6)) and not p:flags("carried") then
      -- Choose ability (only get 1)
      local ability = opt_params.flow_ability
      if not ability then
      local r = rnd.rnd()
         for k,v in ipairs(sirius_abilities_w) do
            if r <= v then
               ability = sirius_abilities[k]
               break
            end
         end
      end
      params.type_range = params.type_range or {}
      if not issirius then
         -- Needs flow structurals
         params.type_range["Flow Amplifier"] = { min=1 }
      end
      params.type_range["Flow Modifier"] = { max=1 }
      p:outfitAdd( ability ) -- Just add the ability here, shouldn't get cleared
      params.noremove = true -- Don't clear outfits
   end

   -- Set some meta-data
   local mem = p:memory()
   mem.equip = { type="sirius", level="elite" }

   -- Try to equip
   return optimize.optimize( p, cores, outfits, params )
end

return equip_sirius
