local optimize = require 'equipopt.optimize'
local eparams = require 'equipopt.params'
local ecores = require 'equipopt.cores'
local eoutfits = require 'equipopt.outfits'

local zalek_outfits = eoutfits.merge{{
   -- Heavy Weapons
   "Za'lek Light Drone Bay", "Za'lek Heavy Drone Bay",
   "Za'lek Bomber Drone Bay",
   "Ragnarok Beam", "Grave Beam",
   -- Medium Weapons
   "Za'lek Heavy Drone Mini Bay", "Za'lek Light Drone Mini Bay",
   "Za'lek Bomber Drone Mini Bay",
   "Enygma Systems Turreted Fury Launcher",
   "Enygma Systems Turreted Headhunter Launcher",
   "Enygma Systems Spearhead Launcher", "TeraCom Fury Launcher",
   "TeraCom Headhunter Launcher", "TeraCom Medusa Launcher",
   "TeraCom Vengeance Launcher",
   "Za'lek Hunter Launcher", "Za'lek Reaper Launcher",
   "Grave Lance", "Orion Beam",
   -- Small Weapons
   "Particle Beam", "Particle Lance",
   -- Utility
   "Droid Repair Crew", "Milspec Scrambler",
   "Targeting Array", "Agility Combat AI",
   "Milspec Jammer", "Emergency Shield Booster",
   "Weapons Ionizer", "Sensor Array",
   "Faraday Tempest Coating", "Hive Combat AI",
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

local zalek_params = {
   ["Za'lek Demon"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,2) },
         },
      } end,
   ["Za'lek Mephisto"] = function () return {
         type_range = {
            ["Launcher"] = { max = rnd.rnd(0,2) },
         },
      } end,
}
local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end
local zalek_cores = {
   ["Za'lek Sting"] = function (_p) return {
         systems = choose_one{ "Milspec Orion 4801 Core System", "Milspec Thalos 4702 Core System" },
         engines = choose_one{ "Tricon Cyclone Engine", "Nexus Arrow 700 Engine", "Melendez Buffalo Engine" },
         hull = choose_one{ "Nexus Ghost Weave", "S&K Battle Plating" },
      } end,
   ["Za'lek Demon"] = function (_p) return {
         systems = choose_one{ "Milspec Orion 4801 Core System", "Milspec Thalos 4702 Core System" },
         systems_secondary = choose_one{ "Milspec Orion 4801 Core System", "Milspec Thalos 4702 Core System" },
         engines = choose_one{ "Tricon Cyclone II Engine", "Nexus Arrow 1400 Engine", "Melendez Buffalo XL Engine" },
         hull = choose_one{ "Nexus Ghost Weave", "S&K Battle Plating" },
         hull_secondary = "S&K Battle Plating",
      } end,
   ["Za'lek Mephisto"] = function (_p) return {
         systems = "Milspec Orion 8801 Core System",
         systems_secondary = "Milspec Orion 8801 Core System",
         engines = choose_one{ "Nexus Bolt 6500 Engine", "Tricon Typhoon II Engine" },
         hull = "Unicorp D-58 Heavy Plating",
         hull_secondary = choose_one{ "Unicorp D-58 Heavy Plating", "Dummy Plating" },
      } end,
   ["Za'lek Diablo"] = function (_p) return {
         systems = "Milspec Thalos 8502 Core System",
         systems_secondary = "Milspec Thalos 8502 Core System",
         hull = "Unicorp D-58 Heavy Plating",
         hull_secondary = choose_one{ "Unicorp D-58 Heavy Plating", "Dummy Plating" },
         engines = choose_one{ "Nexus Bolt 6500 Engine", "Melendez Mammoth XL Engine" },
      } end,
   ["Za'lek Hephaestus"] = function (_p) return {
         systems = "Milspec Thalos 8502 Core System",
         systems_secondary = "Milspec Thalos 8502 Core System",
         hull = choose_one{ "Unicorp D-58 Heavy Plating", "S&K War Plating" }, -- Ideally, should chose twice the same
         hull_secondary = choose_one{ "Unicorp D-58 Heavy Plating", "S&K War Plating" },
         engines = "Melendez Mammoth XL Engine",
      } end,
}

local zalek_params_overwrite = {
   -- Prefer to use the Za'lek utilities
   prefer = {
      ["Hive Combat AI"]          = 100,
      ["Faraday Tempest Coating"] = 100,
   },
   max_same_stru = 3,
}

--[[--
   Does Za'lek pilot equipping

      @param p Pilot to equip
--]]
local function equip_zalek( p, opt_params )
   opt_params = opt_params or {}

   local ps    = p:ship()
   local sname = ps:nameRaw()

   -- Choose parameters and make Za'lekish
   local params = eparams.choose( p, zalek_params_overwrite )
   params.max_mass = 0.95 + 0.2*rnd.rnd()
   -- Per ship tweaks
   local sp = zalek_params[ sname ]
   if sp then
      params = tmerge_r( params, sp() )
   end
   params = tmerge_r( params, opt_params )

   -- Outfits
   local outfits = zalek_outfits
   if opt_params.outfits_add then
      outfits = eoutfits.merge{ outfits, opt_params.outfits_add }
   end

   -- See cores
   local cores = opt_params.cores
   if not cores then
      local zlkcor = zalek_cores[ sname ]
      if zlkcor then
         cores = zlkcor(p)
      else
         cores = ecores.get( p, { all="elite" } )
      end
   end

   -- Set some meta-data
   local mem = p:memory()
   mem.equip = { type="zalek", level="elite" }

   -- Try to equip
   return optimize.optimize( p, cores, outfits, params )
end

return equip_zalek
