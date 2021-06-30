--[[

   Equips pilots based on mixed integer linear programming

--]]
local mt = require 'merge_tables'
local _merge_tables = mt.merge_tables

local equipopt = {}

-- Get all the fighter bays and calculate rough dps
local fbays = {}
for k,o in ipairs(outfit.getAll()) do
   if o:type() == "Fighter Bay" then
      local ss = o:specificstats()
      local s = ss.ship
      local slots = s:getSlots()
      local dps = 0
      for i,sl in ipairs(slots) do
         if sl.type == "Weapon" then
            if sl.size == "Small" then
               dps = dps + 20
            elseif sl.size == "Medium" then
               dps = dps + 25
            elseif sl.size == "Heavy" then
               dps = dps + 30
            end
         end
      end
      dps = dps * ss.amount * 0.5
      fbays[ o:nameRaw() ] = dps
   end
end

-- Some manual overrides
fbays[ "Hyena Fighter Bay" ]                 = 45
fbays[ "Za'lek Light Drone Fighter Bay" ]    = 40
fbays[ "Za'lek Light Drone Fighter Dock" ]   = 80
fbays[ "Za'lek Bomber Drone Fighter Bay" ]   = 40
fbays[ "Za'lek Bomber Drone Fighter Dock" ]  = 80
fbays[ "Za'lek Heavy Drone Fighter Bay" ]    = 50
fbays[ "Za'lek Heavy Drone Fighter Dock" ]   = 100

-- Special weights
local special = {
   ["Enygma Systems Spearhead Launcher"] = 0.4, -- high damage but shield only
   ["TeraCom Medusa Launcher"] = 0.5,           -- really high disable
}


--[[
      Goodness functions to rank how good each outfits are
--]]
function equipopt.goodness_default( o, p )
   -- Base attributes
   base = p.cargo*(0.1*o.cargo + 0.1*(1-o.cargo_inertia)) + p.fuel*0.001*o.fuel
   -- Movement attributes
   move = 0.05*o.thrust + 0.05*o.speed + 0.1*o.turn
   -- Health attributes
   health = 0.01*o.shield + 0.01*o.armour + 0.1*o.shield_regen + 0.1*o.armour_regen + o.absorb
   -- Energy attributes
   energy = 0.003*o.energy + 0.1*o.energy_regen
   -- Weapon attributes
   if o.dps and o.dps > 0 then
      -- Compute damage
      weap = 0.2*(o.dps*p.damage + o.disable*p.disable)
      -- Tracking Modifier
      local mod = math.min( 1, math.max( 0, (p.t_track-o.trackmin)/(o.trackmax-o.trackmin)) )
      -- Range modifier
      mod = mod * math.min( 1, o.range/p.range )
      -- Absorption modifier
      mod = mod * (1 + math.min(0, o.penetration-p.t_absorb))
      -- More modifications
      weap = weap * (0.9*mod+0.1)
      if o.isturret then
         weap = weap * p.turret
      else
         weap = weap * p.forward
      end
      if o.typebroad == "Bolt Weapon" then
         weap = weap * p.bolt
      elseif o.typebroad == "Beam Weapon" then
         weap = weap * p.beam
      elseif o.typebroad == "Launcher" then
         -- Must be able to outrun target
         local smod = math.min( 1, 0.5*(o.spec.speed  / p.t_speed) )
         weap = weap * p.launcher * smod
      elseif o.typebroad == "Fighter Bay" then
         weap = weap * p.fighterbay
      end
   else
      weap = 0
   end
   -- Ewarfare attributes
   ew = 3*(o.ew_detect-1) + 3*(o.ew_hide-1)
   -- Custom weight
   local w = special[o.name] or 1
   local g = p.constant + w*(base + p.move*move + p.health*health + p.energy*energy + p.weap*weap + p.ew*ew)
   --print(string.format("% 32s [%6.3f]: base=%6.3f, move=%6.3f, health=%6.3f, weap=%6.3f, ew=%6.3f", o.name, g * (p.prefer[o.name] or 1), w*base, w*move, w*health, w*weap, w*ew))
    return g * (p.prefer[o.name] or 1)
end

equipopt.params = {}

function equipopt.params.default( overwrite )
   return _merge_tables( {
      -- Our goodness function
      goodness = equipopt.goodness_default,

      -- Global stuff
      constant    = 5, -- Constant value makes them prefer outfits rather than not
      rnd         = 0.2, -- amount of randomness to use for goodness function
      max_same_weap = nil, -- maximum same weapons (nil is no limit)
      max_same_util = nil, -- maximum same utilities (nil is no limit)
      max_same_stru = nil, -- maximum same structurals (nil is no limit)
      min_energy_regen = 0.6, -- relative minimum regen margin (with respect to cores)
      min_energy_regen_abs = 0, -- absolute minimum energy regen (MJ/s)
      eps_weight  = 0.1, -- how to weight weapon EPS into energy regen
      max_mass    = 1.0, -- maximum amount to go over engine limit (relative)
      budget      = nil, -- total cost budget
      -- Range of type, this is dangerous as minimum values could lead to the
      -- optimization problem not having a solution
      type_range  = {
         --["Fighter Bay"] = { min=1, max=2 },
         --["Bolt Turret"] = { min=1, max=2 },
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

function equipopt.params.yacht( overwrite )
   return _merge_tables( equipopt.params.default{
      weap        = 0.1, -- low weapons
      t_absorb    = 0,
      t_speed     = 300,
      t_track     = 4000,
      t_range     = 1000,
   }, overwrite )
end

function equipopt.params.scout( overwrite )
   return _merge_tables( equipopt.params.default{
      weap        = 0.5, -- low weapons
      ew          = 3,
      move        = 2,
      t_absorb    = 0,
      t_speed     = 400,
      t_track     = 4000,
      t_range     = 1000,
   }, overwrite )
end

function equipopt.params.light_fighter( overwrite )
   return _merge_tables( equipopt.params.default{
      move        = 2,
      t_absorb    = 0,
      t_speed     = 400,
      t_track     = 4000,
      t_range     = 1000,
   }, overwrite )
end

function equipopt.params.heavy_fighter( overwrite )
   return _merge_tables( equipopt.params.default{
      t_absorb    = 0.10,
      t_speed     = 300,
      t_track     = 7000,
      t_range     = 1000,
   }, overwrite )
end

function equipopt.params.light_bomber( overwrite )
   return _merge_tables( equipopt.params.default{
      t_absorb    = 0.30,
      t_speed     = 200,
      t_track     = 15e3,
      t_range     = 5000,
      launcher    = 2,
   }, overwrite )
end

function equipopt.params.heavy_bomber( overwrite )
   return _merge_tables( equipopt.params.default{
      t_absorb    = 0.60,
      t_speed     = 50,
      t_track     = 25e3,
      t_range     = 5000,
      launcher    = 2,
   }, overwrite )
end

function equipopt.params.corvette( overwrite )
   return _merge_tables( equipopt.params.default{
      move        = 1.5,
      t_absorb    = 0.20,
      t_speed     = 250,
      t_track     = 10e3,
      t_range     = 3000,
   }, overwrite )
end

function equipopt.params.destroyer( overwrite )
   return _merge_tables( equipopt.params.default{
      t_absorb    = 0.30,
      t_speed     = 150,
      t_track     = 15e3,
      t_range     = 3000,
   }, overwrite )
end

function equipopt.params.light_cruiser( overwrite )
   return _merge_tables( equipopt.params.default{
      t_absorb    = 0.50,
      t_speed     = 100,
      t_track     = 25e3,
      t_range     = 4000,
   }, overwrite )
end

function equipopt.params.heavy_cruiser( overwrite )
   return _merge_tables( equipopt.params.default{
      t_absorb    = 0.70,
      t_speed     = 70,
      t_track     = 35e3,
      t_range     = 4000,
   }, overwrite )
end

function equipopt.params.carrier( overwrite )
   return _merge_tables( equipopt.params.default{
      t_absorb    = 0.50,
      t_speed     = 70,
      t_track     = 35e3,
      t_range     = 4000,
      fighterbay  = 2,
   }, overwrite )
end

-- @brief Chooses a parameter table randomly for a certain pilot p
function equipopt.params.choose( p )
   local choose_table = {
      ["Yacht"] = { "civilian" },
      ["Luxury Yacht"] = { "civilian" },
      ["Cruise Ship"] = { "civilian" },
      ["Drone"] = { "light_fighter", "heavy_fighter" },
      ["Heavy Drone"] = { "corvette" },
      ["Fighter"] = { "light_fighter", "heavy_fighter" },
      ["Bomber"] = { "light_bomber", "heavy_bomber" },
      ["Corvette"] = { "corvette" },
      ["Destroyer"] = { "destroyer" },
      ["Cruiser"] = { "light_cruiser", "heavy_cruiser" },
      ["Carrier"] = { "carrier" },
   }
   local c = choose_table[ p:ship():class() ]
   if not c then
      return equipopt.params.default()
   end
   c = c[ rnd.rnd(1,#c) ]
   return equipopt.params[c]()
end

--[[
      Main equip script.
--]]
function equipopt.equip( p, cores, outfit_list, params )
   params = params or equipopt.params.default()

   -- Determine what outfits from outfit_list we can actually equip
   local ps = p:ship()
   local usable_outfits = {}
   local slots_base = ps:getSlots()
   for m,o in ipairs(outfit_list) do
      for k,v in ipairs( slots_base ) do
         local oo = outfit.get(o)
         local ok = true
         -- Afterburners will be ignored if the ship is too heavy
         if oo:type() == "Afterburner" then
            local st = p:stats()
            local spec = oo:specificstats()
            if spec.mass_limit > 2*st.mass then
               ok = false
            end
         end
         -- Check to see if fits slot
         if ok and ps:fitsSlot( k, o ) then
            table.insert( usable_outfits, o )
            break
         end
      end
   end
   outfit_list = usable_outfits

   -- Naked ship
   p:rmOutfit( "all" )
   if cores then
      -- Don't actually have to remove cores as it should overwrite default
      -- cores as necessary
      --p:rmOutfit( "cores" )
      -- Put cores
      for k,v in ipairs( cores ) do
         p:addOutfit( v, 1, true )
      end
   end

   -- Global ship stuff
   local ss = p:shipstat( nil, true ) -- Should include cores!!
   local st = p:stats() -- also include cores

   -- Optimization problem definition
   local ncols = 0
   local nrows = 0
   local ia = {}
   local ja = {}
   local ar = {}

   -- Figure out limits (both natural and artificial)
   local limit_list = {}
   local same_list = {}
   local same_limit = {}
   for k,v in ipairs(outfit_list) do
      local oo = outfit.get(v)
      -- Add limit if applicable
      local lim = oo:limit()
      if lim then
         limit_list[lim] = true
      end
      -- See if we want to limit the particular outfit
      local t = oo:slot()
      if params.max_same_weap and t=="Weapon" then
         table.insert( same_list, v )
         table.insert( same_limit, params.max_same_weap )
      elseif params.max_same_util and t=="Utility" then
         table.insert( same_list, v )
         table.insert( same_limit, params.max_same_util )
      elseif params.max_same_stru and t=="Structure" then
         table.insert( same_list, v )
         table.insert( same_limit, params.max_same_stru )
      end
   end
   -- Resort limits
   local limits = {}
   for k,v in pairs(limit_list) do
      table.insert( limits, k )
   end
   limit_list = nil

   -- Create outfit cache, it contains all sort of nice information like DPS and
   -- other stuff that can be used for our goodness function
   local outfit_cache = {}
   for k,v in ipairs(outfit_list) do
      local out = outfit.get(v)
      -- Core stats
      local oo = out:shipstat(nil,true)
      oo.outfit   = out
      oo.name     = out:nameRaw()
      oo.dps, oo.disable, oo.eps, oo.range, oo.trackmin, oo.trackmax, oo.lockon = out:weapstats( p )
      oo.trackmin = oo.trackmin or 0
      oo.trackmax = oo.trackmax or 0
      oo.lockon   = oo.lockon or 0
      oo.cpu      = out:cpu()
      oo.mass     = out:mass() * ss.mass_mod
      oo.price    = out:price()
      oo.limit    = out:limit()
      if oo.limit then
         for i,l in ipairs(limits) do
            if l == oo.limit then
               oo.limitpos = i
               break
            end
         end
      end
      oo.type     = out:type()
      oo.spec     = out:specificstats()
      oo.isturret = oo.spec.isturret
      oo.penetration = oo.spec.penetration
      oo.typebroad = out:typeBroad()

      -- We correct ship stats here and convert them to "relative improvements"
      -- Movement
      oo.thrust = oo.thrust_mod * (oo.thrust + st.thrust) - st.thrust
      oo.speed  = oo.speed_mod  * (oo.speed  + st.speed)  - st.speed
      oo.turn   = oo.turn_mod   * (oo.turn   + st.turn)   - st.turn
      -- Health
      oo.armour = oo.armour_mod * (oo.armour + st.armour) - st.armour
      oo.shield = oo.shield_mod * (oo.shield + st.shield) - st.shield
      oo.energy = oo.energy_mod * (oo.energy + st.energy) - st.energy
      oo.armour_regen = oo.armour_regen_mod * (oo.armour_regen + st.armour_regen) - oo.armour_damage - st.armour_regen
      oo.shield_regen = oo.shield_regen_mod * (oo.shield_regen + st.shield_regen) - oo.shield_usage  - st.shield_regen
      oo.energy_regen = oo.energy_regen_mod * (oo.energy_regen + st.energy_regen) - oo.energy_usage  - oo.energy_loss - st.energy_regen
      -- Misc
      oo.cargo = oo.cargo_mod * (oo.cargo + ss.cargo) - ss.cargo

      -- Specific corrections
      if oo.type == "Fighter Bay" then
         -- Fighter bays don't have dps or anything, so we have to fake it
         oo.dps      = fbays[v]
         oo.disable  = 0
         oo.eps      = 0
         oo.range    = 10e3
         oo.penetration = 0
      elseif oo.type == "Afterburners" then
         -- We add it as movement, but weaken the effect a bit
         oo.thrust   = oo.thrust + 0.5*(oo.spec.thrust * st.thrust)
         oo.speed    = oo.speed  + 0.5*(oo.spec.speed * st.speed)
      end

      -- Compute goodness
      oo.goodness = params.goodness( oo, params )

      -- Cache it all so we don't hae to recompute
      outfit_cache[v] = oo
   end

   -- Figure out slots
   local slots = {}
   for k,v in ipairs( slots_base ) do
      local has_outfits = {}
      local outfitpos = {}
      for m,o in ipairs(outfit_list) do
         if ps:fitsSlot( k, o ) then
            table.insert( has_outfits, o )
            -- Check to see if it is in the similar list
            for p,s in ipairs(same_list) do
               if o==s then
                  outfitpos[ #has_outfits ] = p
                  break
               end
            end
         end
      end

      if #has_outfits > 0 then
         v.id = k
         v.outfits = has_outfits
         v.samepos = outfitpos
         --[[
         print( string.format( "[%d] = %s, %s", k, v.type, v.size ) )
         for m, o in ipairs(v.outfits) do
            print( "   "..o )
         end
         --]]
         table.insert( slots, v )

         -- Each slot adds a number of variables equivalent to the number of
         -- potential outfits, but only one constraint
         ncols = ncols + #v.outfits
         nrows = nrows + 1
      end
   end

   -- We have to add additional constraints (spaceworthy, limits)
   local sworthy = 3 -- Check CPU, energy regen, and mass
   if params.budget then
      sworthy = sworthy + 1
   end
   nrows = nrows + sworthy + #limits
   if #same_list > 0 then
      nrows = nrows + #same_list
   end
   local ntype_range = 0
   for k,v in pairs(params.type_range) do ntype_range = ntype_range+1 end
   nrows = nrows + ntype_range
   lp = linopt.new( "test", ncols, nrows, true )
   -- Add space worthy checks
   lp:set_row( 1, "CPU",          nil, st.cpu_max * ss.cpu_mod )
   lp:set_row( 2, "energy_regen", nil, st.energy_regen - math.max(params.min_energy_regen*st.energy_regen, params.min_energy_regen_abs) )
   lp:set_row( 3, "mass",         nil, params.max_mass * ss.engine_limit - st.mass )
   if params.budget then
      lp:set_row( 4, "budget",    nil, params.budget )
   end
   -- Add limit checks
   for i,l in ipairs(limits) do
      lp:set_row( sworthy+i, l, nil, 1 )
   end
   local nsame = 0
   if #same_list > 0 then
      for i,o in ipairs(same_list) do
         lp:set_row( sworthy+#limits+i, o, nil, same_limit[i] )
      end
      nsame = #same_list
   end
   local r = sworthy+#limits+nsame+1
   for name,v in pairs(params.type_range) do
      v.id = r
      lp:set_row( v.id, name, v.min, v.max )
      r = r+1
   end
   -- Add outfit checks
   local c = 1
   for i,s in ipairs(slots) do
      for j,o in ipairs(s.outfits) do
         local stats = outfit_cache[o]
         local name = string.format("s%d-o%d", i, j)
         local objf = (1+params.rnd*rnd.sigma()) * stats.goodness -- contribution to objective function
         lp:set_col( c, name, objf, "binary" ) -- constraints set automatically
         -- CPU constraint
         table.insert( ia, 1 )
         table.insert( ja, c )
         table.insert( ar, -stats.cpu )
         -- Energy constraint
         table.insert( ia, 2 )
         table.insert( ja, c )
         table.insert( ar, -stats.energy_regen + params.eps_weight*(stats.eps or 0) )
         -- Mass constraint
         table.insert( ia, 3 )
         table.insert( ja, c )
         table.insert( ar, stats.mass )
         -- Budget constraint if necessary
         if params.budget then
            table.insert( ia, 4 )
            table.insert( ja, c )
            table.insert( ar, stats.price )
         end
         -- Limit constraint
         if stats.limit then
            table.insert( ia, sworthy + stats.limitpos )
            table.insert( ja, c )
            table.insert( ar, 1 )
         end
         -- Only one outfit per slot constraint
         table.insert( ia, r )
         table.insert( ja, c )
         table.insert( ar, 1 )
         -- Maximum of same type
         local sp = s.samepos[j]
         if sp then
            table.insert( ia, sworthy + #limits + sp )
            table.insert( ja, c )
            table.insert( ar, 1 )
         end
         -- Check type range
         if ntype_range > 0 then
            local r = params.type_range[ stats.name ]
            if rn then
               table.insert( ia, r.id )
               table.insert( ja, c )
               table.insert( ar, 1 )
            end
            local r = params.type_range[ stats.type ]
            if r then
               table.insert( ia, r.id )
               table.insert( ja, c )
               table.insert( ar, 1 )
            end
            if stats.type ~= stats.typebroad then
               local r = params.type_range[ stats.typebroad ]
               if r then
                  table.insert( ia, r.id )
                  table.insert( ja, c )
                  table.insert( ar, 1 )
               end
            end
         end
         c = c + 1
      end
      lp:set_row( r, string.format("s%d-sum", i), nil, 1 )
      r = r + 1
   end

   --[[
   local M = {}
   for i = 1,nrows do
      M[i] = {}
      for j = 1,ncols do
         M[i][j] = 0
      end
   end
   for i = 1,#ia do
      M[ ia[i] ][ ja[i] ] = ar[i]
   end
   for i = 1,nrows do
      s = ""
      for j = 1,ncols do
         s = s .. string.format("% 4d", M[i][j])
      end
      print(s)
   end
   --]]

   -- All the magic is done here
   lp:load_matrix( ia, ja, ar )
   z, x, c = lp:solve()
   if not z then
      -- Maybe should be error instead?
      warn(string.format("Failed to solve linear program: %s", x))
      return nil
   end
   --[[
   for k,v in ipairs(x) do
      print(string.format("x%d: %d",k,v))
   end
   for k,v in ipairs(c) do
      print(string.format("c%d: %d",k,v))
   end
   --]]

   -- Interpret results
   --print("Final Equipment:")
   local c = 1
   for i,s in ipairs(slots) do
      for j,o in ipairs(s.outfits) do
         if x[c] == 1 then
            p:addOutfit( o, 1, true )
            --print( "   "..o )
         end
         c = c + 1
      end
   end

   -- Fill ammo
   p:fillAmmo()

   -- Check
   if __debugging then
      local b, s = p:spaceworthy()
      if not b then
         warn(string.format(_("Pilot '%s' is not space worthy after equip script is run! Reason: %s"),p:name(),s))
      end
   end
end

return equipopt
