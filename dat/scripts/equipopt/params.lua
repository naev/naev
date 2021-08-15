local mt = require 'merge_tables'
local _merge_tables = mt.merge_tables

local params = {}

function params.default( overwrite )
   return _merge_tables( {
      -- Global stuff
      constant    = 10, -- Constant value makes them prefer outfits rather than not
      rnd         = 0.2, -- amount of randomness to use for goodness function
      max_same_weap = nil, -- maximum same weapons (nil is no limit)
      max_same_util = nil, -- maximum same utilities (nil is no limit)
      max_same_stru = nil, -- maximum same structurals (nil is no limit)
      min_energy_regen = 0.6, -- relative minimum regen margin (with respect to cores)
      min_energy_regen_abs = 0, -- absolute minimum energy regen (MJ/s)
      eps_weight  = 0.4, -- how to weight weapon EPS into energy regen
      max_mass    = 1.2, -- maximum amount to go over engine limit (relative)
      min_mass_margin = 0.15, -- minimum mass margin to consider when equipping
      budget      = nil, -- total cost budget
      -- Range of type, this is dangerous as minimum values could lead to the
      -- optimization problem not having a solution with high minimums
      type_range  = {
         ["Launcher"] = { max=2 },
      },
      -- Outfit names that the pilot should prefer (multiplies weights)
      prefer = {
         --["Hive Combat AI"] = 100,
      },

      -- High level weights
      move        = 1,
      health      = 1,
      energy      = 1,
      weap        = 1,
      ew          = 1,
      -- Not as important
      cargo       = 1,
      fuel        = 1,

      -- Weapon stuff
      t_absorb    = 0.2, -- assumed target absorption
      t_speed     = 250, -- assumed target speed
      t_track     = 10000, -- ew_detect enemies we want to target
      range       = 2000, -- ideal minimum range we want
      damage      = 1, -- weight for normal damage
      disable     = 1, -- weight for disable damage
      turret      = 1,
      forward     = 1,
      launcher    = 1,
      beam        = 1,
      bolt        = 1,
      fighterbay  = 1,
   }, overwrite )
end

function params.civilian( overwrite )
   return _merge_tables( params.default{
      weap        = 0.5, -- low weapons
      t_absorb    = 0,
      t_speed     = 300,
      t_track     = 4000,
      t_range     = 1000,
   }, overwrite )
end

function params.merchant( overwrite )
   return _merge_tables( params.default{
      weap        = 0.5, -- low weapons
      t_absorb    = 0,
      t_speed     = 300,
      t_track     = 4000,
      t_range     = 1000,
      cargo       = 2,
      forward     = 0.5, -- Less forward weapons
   }, overwrite )
end

function params.armoured_transport( overwrite )
   return _merge_tables( params.default{
      t_absorb    = 0,
      t_speed     = 300,
      t_track     = 4000,
      t_range     = 1000,
      cargo       = 1.5,
      forward     = 0.3, -- Less forward weapons
   }, overwrite )
end

function params.scout( overwrite )
   return _merge_tables( params.default{
      weap        = 0.5, -- low weapons
      ew          = 2,
      t_absorb    = 0,
      t_speed     = 400,
      t_track     = 4000,
      t_range     = 1000,
   }, overwrite )
end

function params.light_fighter( overwrite )
   return _merge_tables( params.default{
      eps_weight  = 0.2,
      t_absorb    = 0,
      t_speed     = 400,
      t_track     = 4000,
      t_range     = 1000,
   }, overwrite )
end

function params.heavy_fighter( overwrite )
   return _merge_tables( params.default{
      eps_weight  = 0.3,
      t_absorb    = 0.10,
      t_speed     = 300,
      t_track     = 7000,
      t_range     = 1000,
   }, overwrite )
end

function params.light_bomber( overwrite )
   return _merge_tables( params.default{
      t_absorb    = 0.30,
      t_speed     = 200,
      t_track     = 15e3,
      t_range     = 5000,
      launcher    = 2,
      type_range  = {
         ["Launcher"] = { max=3 },
      },
   }, overwrite )
end

function params.heavy_bomber( overwrite )
   return _merge_tables( params.default{
      t_absorb    = 0.60,
      t_speed     = 50,
      t_track     = 25e3,
      t_range     = 5000,
      launcher    = 2,
      type_range  = {
         ["Launcher"] = { max=3 },
      },
   }, overwrite )
end

function params.corvette( overwrite )
   return _merge_tables( params.default{
      move        = 1.5,
      t_absorb    = 0.20,
      t_speed     = 250,
      t_track     = 10e3,
      t_range     = 3000,
      type_range  = {
         ["Launcher"] = { max=3 },
      },
   }, overwrite )
end

function params.destroyer( overwrite )
   return _merge_tables( params.default{
      t_absorb    = 0.30,
      t_speed     = 150,
      t_track     = 15e3,
      t_range     = 3000,
   }, overwrite )
end

function params.light_cruiser( overwrite )
   return _merge_tables( params.default{
      t_absorb    = 0.50,
      t_speed     = 130,
      t_track     = 20e3,
      t_range     = 4000,
   }, overwrite )
end

function params.heavy_cruiser( overwrite )
   return _merge_tables( params.default{
      t_absorb    = 0.70,
      t_speed     = 70,
      t_track     = 35e3,
      t_range     = 4000,
   }, overwrite )
end

function params.carrier( overwrite )
   return _merge_tables( params.default{
      t_absorb    = 0.50,
      t_speed     = 70,
      t_track     = 35e3,
      t_range     = 5000,
      fighterbay  = 2,
   }, overwrite )
end

-- @brief Chooses a parameter table randomly for a certain pilot p
function params.choose( p )
   local choose_table = {
      ["Courier"]       = { "merchant" },
      ["Freighter"]     = { "merchant" },
      ["Bulk Freighter"]= { "merchant" },
      ["Armoured Transport"] = { "armoured_transport" },
      ["Interceptor"]   = { "light_fighter" },
      ["Fighter"]       = { "heavy_fighter" },
      ["Bomber"]        = { "light_bomber", "heavy_bomber" },
      ["Corvette"]      = { "corvette" },
      ["Destroyer"]     = { "destroyer" },
      ["Cruiser"]       = { "light_cruiser" },
      ["Battleship"]    = { "heavy_cruiser" },
      ["Carrier"]       = { "carrier" },
   }
   local c = choose_table[ p:ship():class() ]
   if not c then
      return params.default()
   end
   c = c[ rnd.rnd(1,#c) ]
   return params[c]()
end

return params
