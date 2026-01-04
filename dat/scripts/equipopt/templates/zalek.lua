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
   "Particle Beam", "Particle Lance", "Orion Lance", "Electron Burst Cannon",
   -- Point Defence
   "ZIBS-16", "ZIBS-32",
   -- Utility
   "Droid Repair Crew", "Milspec Scrambler", "Unicorp Scrambler",
   "Targeting Array", "Agility Combat AI", "Unicorp Jammer", "Hyperbolic Blink Engine",
   "Milspec Jammer", "Emergency Shield Booster", "Weakness Harmonizer AI",
   "Weapons Ionizer", "Sensor Array", "Sensor Array", "Flicker Drive",
   "Faraday Tempest Coating", "Hive Combat AI",
   -- Heavy Structural
   "Battery III", "Shield Capacitor III", "Shield Capacitor IV",
   "Reactor Class III", "Battery IV", "Auxiliary Processing Unit IV",
   "Large Shield Booster", "Auxiliary Processing Unit III",
   -- Medium Structural
   "Battery II", "Shield Capacitor II", "Reactor Class II",
   "Medium Shield Booster", "Auxiliary Processing Unit II",
   -- Small Structural
   "Improved Stabilizer", "Engine Reroute",
   "Battery I", "Shield Capacitor I", "Reactor Class I",
   "Small Shield Booster", "Auxiliary Processing Unit I",
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
         systems = choose_one{ "Milspec Aegis 4701 Core System", "Milspec Thalos 4702 Core System" },
         engines = choose_one{ "Tricon Cyclone Engine", "Nexus Arrow 700 Engine", "Melendez Buffalo Engine" },
         hull = choose_one{ "Nexus Ghost Weave", "S&K Battle Plating" },
      } end,
   ["Za'lek Demon"] = function (_p) return {
         systems = choose_one{ "Milspec Aegis 4701 Core System", "Milspec Thalos 4702 Core System" },
         systems_secondary = choose_one{ "Milspec Aegis 4701 Core System", "Milspec Thalos 4702 Core System" },
         engines = choose_one{ "Tricon Cyclone Engine", "Nexus Arrow 700 Engine", "Melendez Buffalo Engine" },
         engines_secondary = choose_one{ "Tricon Cyclone Engine", "Nexus Arrow 700 Engine", "Melendez Buffalo Engine" },
         hull = choose_one{ "Nexus Ghost Weave", "S&K Battle Plating" },
         hull_secondary = "S&K Battle Plating",
      } end,
   ["Za'lek Mephisto"] = function (_p) return {
         systems = choose_one{ "Milspec Aegis 8501 Core System", "Milspec Thalos 8502 Core System" },
         systems_secondary = choose_one{ "Milspec Aegis 8501 Core System", "Milspec Thalos 8502 Core System" },
         engines = choose_one{ "Nexus Bolt 3000 Engine", "Tricon Typhoon Engine" },
         engines_secondary = choose_one{ "Nexus Bolt 3000 Engine", "Tricon Typhoon Engine" },
         hull = "Unicorp D-58 Heavy Plating",
         --hull_secondary = choose_one{ "Unicorp D-58 Heavy Plating", "Dummy Plating" },
         hull_secondary = "Unicorp D-58 Heavy Plating",
      } end,
   ["Za'lek Diablo"] = function (_p) return {
         systems = choose_one{ "Milspec Aegis 8501 Core System", "Milspec Thalos 8502 Core System" },
         systems_secondary = choose_one{ "Milspec Aegis 8501 Core System", "Milspec Thalos 8502 Core System" },
         hull = "Unicorp D-58 Heavy Plating",
         --hull_secondary = choose_one{ "Unicorp D-58 Heavy Plating", "Dummy Plating" },
         hull_secondary = "Unicorp D-58 Heavy Plating",
         engines = choose_one{ "Nexus Bolt 3000 Engine", "Melendez Mammoth Engine" },
         engines_secondary = choose_one{ "Nexus Bolt 3000 Engine", "Melendez Mammoth Engine" },
      } end,
   ["Za'lek Hephaestus"] = function (_p) return {
         systems = choose_one{ "Milspec Aegis 8501 Core System", "Milspec Thalos 8502 Core System" },
         systems_secondary = choose_one{ "Milspec Aegis 8501 Core System", "Milspec Thalos 8502 Core System" },
         hull = choose_one{ "Unicorp D-58 Heavy Plating", "S&K War Plating" }, -- Ideally, should chose twice the same
         hull_secondary = choose_one{ "Unicorp D-58 Heavy Plating", "S&K War Plating" },
         engines = "Melendez Mammoth Engine",
         engines_secondary = "Melendez Mammoth Engine",
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
      cores = ecores.get( p, { all="elite" } )
      if zlkcor then
         cores = tmerge( cores, zlkcor(p) )
      end
   end

   -- Set some meta-data
   local mem = p:memory()
   mem.equip = { type="zalek", level="elite" }

   -- Try to equip
   return optimize.optimize( p, cores, outfits, params )
end

return equip_zalek
