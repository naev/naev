local params = {}

function params.default( overwrite )
   return tmerge( {
      -- Global stuff
      constant    = 10, -- Constant value makes them prefer outfits rather than not
      rnd         = 0.2, -- amount of randomness to use for goodness function
      max_weap    = nil, -- Maximum number of ewapon slots to use
      max_util    = nil, -- Maximum number of utility slots to use
      max_stru    = nil, -- Maximum number of structural slots to use
      max_same_weap = nil, -- maximum same weapons (nil is no limit)
      max_same_util = nil, -- maximum same utilities (nil is no limit)
      max_same_stru = nil, -- maximum same structurals (nil is no limit)
      min_energy_regen = 0.6, -- relative minimum regen margin (with respect to cores)
      min_energy_regen_abs = 0, -- absolute minimum energy regen (GW)
      eps_weight  = 0.4, -- how to weight weapon EPS into energy regen
      max_mass    = 1.2, -- maximum amount to go over engine limit (relative)
      min_mass_margin = 0.15, -- minimum mass margin to consider when equipping
      budget      = nil, -- total cost budget
      -- Range of type, this is dangerous as minimum values could lead to the
      -- optimization problem not having a solution with high minimums
      type_range  = {
         ["Launcher"] = { max=2 }, -- typebroad
         ["Point Defense"] = { max=2 }, -- typename
      },
      -- Outfit names that the pilot should prefer (multiplies weights)
      prefer = {
         --["Hive Combat AI"] = 100,
      },
      mismatch    = 0.5, -- Penalty for slot size mismatch

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
      t_track     = 10e3, -- ew_signature enemies we want to target
      duration    = 15, -- estimated fight time duration
      range       = 2e3, -- ideal minimum range we want
      damage      = 1, -- weight for normal damage
      disable     = 1, -- weight for disable damage
      turret      = 1,
      forward     = 1,
      launcher    = 1,
      beam        = 1,
      bolt        = 1,
      fighterbay  = 1,
      seeker      = 1,
   }, overwrite )
end

function params.civilian( overwrite )
   return tmerge( params.default{
      weap        = 0.5, -- low weapons
      t_absorb    = 0,
      t_speed     = 300,
      t_track     = 4e3,
      t_range     = 1e3,
   }, overwrite )
end

function params.merchant( overwrite )
   return tmerge( params.default{
      weap        = 0.5, -- low weapons
      t_absorb    = 0,
      t_speed     = 300,
      t_track     = 4e3,
      t_range     = 1e3,
      cargo       = 2,
      forward     = 0.5, -- Less forward weapons
      prefer      = {
         ["Point Defense"] = 1.1,
         ["Fighter Bay"] = 1.1,
         ["Bolt Cannon"] = 0.9,
         ["Beam Cannon"] = 0.9,
      },
   }, overwrite )
end

function params.armoured_transport( overwrite )
   return tmerge( params.default{
      t_absorb    = 0,
      t_speed     = 300,
      t_track     = 4e3,
      t_range     = 1e3,
      cargo       = 1.5,
      forward     = 0.3, -- Less forward weapons
   }, overwrite )
end

function params.scout( overwrite )
   return tmerge( params.default{
      weap        = 0.5, -- low weapons
      ew          = 2,
      t_absorb    = 0,
      t_speed     = 400,
      t_track     = 4e3,
      t_range     = 1e3,
   }, overwrite )
end

function params.interceptor( overwrite )
   return tmerge( params.default{
      eps_weight  = 0.2,
      t_absorb    = 0,
      t_speed     = 400,
      t_track     = 4e3,
      t_range     = 1e3,
      duration    = 5,
   }, overwrite )
end

function params.fighter( overwrite )
   return tmerge( params.default{
      eps_weight  = 0.3,
      t_absorb    = 0.10,
      t_speed     = 300,
      t_track     = 7e3,
      t_range     = 1e3,
      duration    = 10,
   }, overwrite )
end

function params.light_bomber( overwrite )
   return tmerge( params.default{
      t_absorb    = 0.30,
      t_speed     = 300,
      t_track     = 10e3,
      t_range     = 5e3,
      duration    = 20,
      seeker      = 1.5,
      type_range  = {
         ["Launcher"] = { max=3 },
      },
   }, overwrite )
end

function params.medium_bomber( overwrite )
   return tmerge( params.default{
      t_absorb    = 0.50,
      t_speed     = 200,
      t_track     = 20e3,
      t_range     = 5e3,
      duration    = 30,
      seeker      = 1.5,
      type_range  = {
         ["Launcher"] = { max=3 },
      },
   }, overwrite )
end

function params.heavy_bomber( overwrite )
   return tmerge( params.default{
      t_absorb    = 0.80,
      t_speed     = 50,
      t_track     = 30e3,
      t_range     = 6e3,
      duration    = 50,
      seeker      = 1.5,
      type_range  = {
         ["Launcher"] = { max=3 },
      },
   }, overwrite )
end

function params.corvette( overwrite )
   return tmerge( params.default{
      move        = 1.5,
      t_absorb    = 0.20,
      t_speed     = 250,
      t_track     = 10e3,
      t_range     = 3e3,
      type_range  = {
         ["Launcher"] = { max=3 },
      },
   }, overwrite )
end

function params.destroyer( overwrite )
   return tmerge( params.default{
      t_absorb    = 0.30,
      t_speed     = 150,
      t_track     = 15e3,
      t_range     = 2e3,
      duration    = 25,
   }, overwrite )
end

function params.cruiser( overwrite )
   return tmerge( params.default{
      t_absorb    = 0.50,
      t_speed     = 130,
      t_track     = 20e3,
      t_range     = 2.5e3,
      duration    = 30,
   }, overwrite )
end

function params.battleship( overwrite )
   return tmerge( params.default{
      t_absorb    = 0.80,
      t_speed     = 70,
      t_track     = 25e3,
      t_range     = 2.5e3,
      duration    = 40,
      launcher    = 0.25,
      mismatch    = 0.25,
   }, overwrite )
end

function params.carrier( overwrite )
   return tmerge( params.default{
      t_absorb    = 0.50,
      t_speed     = 70,
      t_track     = 25e3,
      t_range     = 5e3,
      fighterbay  = 1.5,
      duration    = 50,
      mismatch    = 0.25,
   }, overwrite )
end

-- @brief Chooses a parameter table randomly for a certain pilot p
function params.choose( p, overwrite )
   local choose_table = {
      ["Courier"]       = { "merchant" },
      ["Freighter"]     = { "merchant" },
      ["Bulk Freighter"]= { "merchant" },
      ["Armoured Transport"] = { "armoured_transport" },
      ["Interceptor"]   = { "interceptor" },
      ["Fighter"]       = { "fighter" },
      ["Bomber"]        = { "light_bomber", "medium_bomber", "heavy_bomber" },
      ["Corvette"]      = { "corvette" },
      ["Destroyer"]     = { "destroyer" },
      ["Cruiser"]       = { "cruiser" },
      ["Battleship"]    = { "battleship" },
      ["Carrier"]       = { "carrier" },
   }
   local c = choose_table[ p:ship():class() ]
   if not c then
      return params.default()
   end
   c = c[ rnd.rnd(1,#c) ]
   return params[c]( overwrite )
end

return params
