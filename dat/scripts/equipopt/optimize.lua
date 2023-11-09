--[[--
   Equips pilots based on mixed integer linear programming.

   @module equipopt.optimize
--]]
local optimize = {}
local eparams = require 'equipopt.params'
local bioship = require 'bioship'
local ai_setup = require "ai.core.setup"
local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

-- Create caches and stuff
-- Get all the fighter bays and calculate rough dps
local outfit_stats = {}
local fbay_dps = {}
for k,o in ipairs(outfit.getAll()) do
   local os = o:shipstat(nil,true)
   os.type = o:type()
   os.typebroad = o:typeBroad()
   os.spec = o:specificstats()
   os.price = o:price()
   os.limit = o:limit()
   os.mass = o:mass()
   os.cpu = o:cpu()
   outfit_stats[o:nameRaw()] = os
   if os.type == "Fighter Bay" then
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
      dps = dps * ss.amount
      fbay_dps[ o:nameRaw() ] = 5*math.sqrt(dps)
   end
end

-- Manual overrides
local goodness_override = {
   -- Combat AIs
   ["Hunting Combat AI"]   = 3,
   ["Hive Combat AI"]      = 3,
   ["Pinpoint Combat AI"]  = 3,
   ["Cyclic Combat AI"]    = 3,
   ["Scanning Combat AI"]  = 3,
   ["Agility Combat AI"]   = 3,
   ["Bio-Neural Combat AI"]= 3,
   -- Hull Coating
   ["Milspec Impacto-Plastic Coating"] = 3,
   ["Photo-Voltaic Nanobot Coating"] = 3,
   ["Nexus Stealth Coating"] = 3,
   ["Lattice Thermal Coating"] = 3,
   ["Faraday Tempest Coating"] = 3,
   ["Nebula Resistant Coating"] = 3,
   -- Utility Stuff
   ["Berserk Chip"] = 3,
   ["Neural Accelerator Interface"] = 3,
   ["Emergency Stasis Inducer"] = 3,
   ["Combat Hologram Projector"] = 3,
   ["Weakness Harmonizer AI"] = 3,
   ["Weapons Ionizer"] = 3,
   ["Boarding Androids MK1"] = 2,
   ["Boarding Androids MK2"] = 4,
   ["Hidden Jump Scanner"] = 0,
   ["Asteroid Scanner"] = 1,
   ["Blink Drive"] = 3,
   ["Hyperbolic Blink Engine"] = 3,
}

-- Special weights
local goodness_special = {
   ["Black Cat Doll"] = 0.25, -- Only active when stealthing
   ["TeraCom Mace Launcher"] = 0.5,
   ["Enygma Systems Huntsman Launcher"] = 0.5,
   ["Enygma Systems Spearhead Launcher"] = 0.4, -- high damage but shield only
   ["TeraCom Medusa Launcher"] = 0.5,           -- really high disable
   ["Droid Repair Crew"] = 0.4, -- Only work until 50%
   ["Electron Burst Cannon"] = 0.7, -- Shieldbreaker damage
   -- Plasma do a lot of damage over time
   ["Plasma Blaster MK1"] = 1 / 0.75,
   ["Plasma Blaster MK2"] = 1 / 0.75,
   ["Plasma Cannon"] = 1 / 0.75,
   ["Plasma Cluster Cannon"] = 1 / 0.75,
   ["Plasma Turret MK1"] = 1 / 0.75,
   ["Plasma Turret MK2"] = 1 / 0.75,
   -- Razor/Disruptors do disable over time (and energy drain!)
   ["Razor Artillery S1"] = 1 / 0.75,
   ["Razor Artillery S2"] = 1 / 0.75,
   ["Razor Artillery S3"] = 1 / 0.75,
   ["Razor Battery S3"] = 1 / 0.75,
   ["Disruptor Artillery S1"] = 1 / 0.75,
   ["Disruptor Artillery S2"] = 1 / 0.75,
   ["Disruptor Battery S3"] = 1 / 0.75,
}


--[[
      Completely custom ship builds: they do not use optimization
--]]
local special_ships = {}
special_ships["Drone"] = function( p )
   for k,o in ipairs{
      "Milspec Orion 2301 Core System",
      "Nexus Dart 160 Engine",
      "Nexus Light Stealth Plating",
      "Neutron Disruptor",
      "Neutron Disruptor",
      "Neutron Disruptor",
   } do
      p:outfitAdd( o, 1, true )
   end
end
special_ships["Heavy Drone"] = function( p )
   for k,o in ipairs{
      "Milspec Thalos 3602 Core System",
      "Unicorp Hawk 360 Engine",
      choose_one{"Nexus Light Stealth Plating", "S&K Light Combat Plating"},
      "Shatterer Launcher",
      "Shatterer Launcher",
      "Heavy Neutron Disruptor",
      "Heavy Neutron Disruptor",
   } do
      p:outfitAdd( o, 1, true )
   end
end


--[[
      Goodness functions to rank how good each outfits are
--]]
function optimize.goodness_default( o, p )
   local os = o.stats
   -- Base attributes
   local base = p.cargo*(0.5*math.pow(o.cargo,0.3) + 0.1*(1-os.cargo_inertia)) + p.fuel*0.003*os.fuel
   -- Movement attributes
   local move = 0.1*o.accel + 0.1*o.speed + 0.2*o.turn + 50*(os.time_speedup-1)
   -- Health attributes
   local health = 0.01*o.shield + 0.02*o.armour + 0.9*o.shield_regen + 2*o.armour_regen + os.absorb/10
   -- Energy attributes
   local energy = 0.003*o.energy + 0.18*o.energy_regen
   -- Weapon attributes
   local weap = 0
   if o.dps and o.dps > 0 then
      -- Compute damage
      weap = 0.5*(o.dps*p.damage + o.disable*p.disable)
      -- Tracking Modifier
      local mod = math.min( 1, math.max( 0, (p.t_track-o.trackmin)/(1+o.trackmax-o.trackmin)) )
      -- Range modifier
      mod = mod * math.min( 1, o.range/p.range )
      -- Absorption modifier
      mod = mod * (1 + math.min(0, o.penetration-p.t_absorb))
      -- Launcher modification
      mod = mod * math.max( 0, (p.duration - o.lockon) / p.duration )
      -- More modifications
      weap = weap * (0.9*mod+0.1)
      if o.isturret then
         weap = weap * p.turret
      else
         weap = weap * p.forward
      end
      if o.seeker then
         weap = weap * p.seeker
      end
      if o.typebroad == "Bolt Weapon" then
         weap = weap * p.bolt
      elseif o.typebroad == "Beam Weapon" then
         weap = weap * p.beam
      elseif o.typebroad == "Launcher" then
         -- Must be able to outrun target
         local smod = math.min( 1, 0.33*(o.spec.speed_max / p.t_speed) )
         weap = weap * p.launcher * smod
      elseif o.typebroad == "Fighter Bay" then
         weap = weap * p.fighterbay
      end
   end
   -- Ewarfare attributes
   local ew = 3*(1-1/os.ew_detect) + 3*(1-1/os.ew_hide)
   -- Custom weight
   local w = goodness_special[o.name] or 1
   local g = p.constant + w*(base + p.move*move + p.health*health + p.energy*energy + p.weap*weap + p.ew*ew)
   --print(string.format("% 32s [%6.3f]: base=%6.3f, move=%6.3f, health=%6.3f, weap=%6.3f, ew=%6.3f", o.name, g * (p.prefer[o.name] or 1), w*base, w*move, w*health, w*weap, w*ew))
   return g * (p.prefer[o.name] or p.prefer[o.type] or 1)
end


local function print_debug( p, st, ss, outfit_list, params, constraints, energygoal, emod, mmod, nebu_row, _budget_row )
   -- TODO: displaying budget_row could be useful.
   emod = emod or 1
   mmod = mmod or 1
   print(_("Trying to equip:"))
   for j,o in ipairs(outfit_list) do
      print( "   "..o:name() )
   end
   print(_("Parameters:"))
   for k,v in pairs(params) do
      if type(v)=="table" then
         print(string.format("   %s:", k ))
         for i,m in pairs(v) do
            print(string.format("      %s: %s", i, m ))
         end
      else
         print(string.format("   %s: %s", k, v ))
      end
   end
   print(_("Equipment:"))
   for j,o in ipairs(p:outfitsList()) do
      print( "   "..o:name() )
   end
   local stn = p:stats()
   constraints = constraints or {}
   print(string.format(_("CPU: %d / %d [%d < %d]"), stn.cpu, stn.cpu_max, constraints[1] or 0, st.cpu_max * ss.cpu_mod ))
   print(string.format(_("Energy Regen: %.3f [%.3f < %.3f (%.1f)]"), stn.energy_regen, constraints[2] or 0, st.energy_regen - emod*energygoal, emod))
   print(string.format(_("Mass: %.3f / %.3f [%.3f < %.3f (%.1f)]"), st.mass, ss.engine_limit, constraints[3] or 0, mmod * params.max_mass * ss.engine_limit - st.mass, mmod ))
   if nebu_row then
      local _nebu_dens, nebu_vol = system.cur():nebula()
      print(string.format(_("Shield Regen: %.3f [%.3f > %.3f (%.1f)]"), stn.shield_regen, constraints[nebu_row] or 0, nebu_vol*(1-ss.nebu_absorb)-st.shield_regen, 1))
   end
end

--[[--
   Equips a pilot with cores and outfits chosen from a list through optimization.

      @tparam Pilot p Pilot to equip.
      @tparam[opt=nil] table|nil cores Table of core outfits (by name) to equip. They will replace existing outfits, or set to nil to use defaults.
      @tparam table outfit_list List of outfits to try to equip (by name). There can be duplicates in the list, and only outfits that can be equipped are considered.
      @tparam[opt=nil] table|nil params Parameter list to use or nil for defaults.
      @treturn boolean Whether or not the pilot was properly equipped
--]]
function optimize.optimize( p, cores, outfit_list, params )
   params = params or eparams.default()
   params.goodness = params.goodness or optimize.goodness_default
   local sparams = optimize.sparams
   local pm = p:memory()
   pm.equipopt_params = params

   -- Naked ship
   local ps = p:ship()
   local pt = ps:tags()
   if pt.noequip then -- Don't equip
      -- Set up useful outfits
      ai_setup.setup(p)
      return
   end
   if not params.noremove then
      p:outfitRm( "all" )
   end

   -- Special ships used fixed outfits
   local specship = special_ships[ ps:nameRaw() ]
   if specship then
      specship( p )
      if __debugging then
         local b, s = p:spaceworthy()
         if not b then
            warn(string.format(_("Pilot '%s' is not space worthy after custom equip script is run! Reason: %s"),p:name(),s))
         end
         return false
      end
      -- Set up useful outfits
      ai_setup.setup(p)
      return true
   end

   -- Special case bioships
   if pt.bioship and not p:shipvarPeek("bioship_init") then
      local stage = bioship.maxstage( p )
      bioship.simulate( p, rnd.rnd(1,stage) )
   end

   -- Handle cores
   if cores and not pt.nocores then
      -- Don't actually have to remove cores as it should overwrite default
      -- cores as necessary
      --p:outfitRm( "cores" )
      -- Put cores
      for k,v in ipairs( cores ) do
         local q = p:outfitAdd( v, 1, true )
         if q < 1 then
            warn(string.format(_("Unable to equip core '%s' on '%s'!"), v, p:name()))
         end
      end
   end

   -- Global ship stuff
   local ss = p:shipstat( nil, true ) -- Should include cores!!
   local st = p:stats() -- also include cores

   -- Modify forward weapon bonus depending on turn rate
   if st.turn < 150 then
      params.forward = params.forward * math.max( 0.5, st.turn/150 )
   end

   -- Determine what outfits from outfit_list we can actually equip
   -- We actually remove duplicates too
   local usable_outfits = {}
   local slots_base = ps:getSlots()
   for m,o in ipairs(outfit_list) do
      if not usable_outfits[o] then
         local ok = true
         -- Afterburners will be ignored if the ship is too heavy
         if o:type() == "Afterburner" then
            local spec = o:specificstats()
            if spec.mass_limit < 0.8*ss.engine_limit then
               ok = false
            end
         end
         if ok then
            for k,v in ipairs(slots_base) do
               -- Check to see if fits slot
               if ps:fitsSlot( k, o ) then
                  usable_outfits[o] = true
                  break
               end
            end
         end
      end
   end
   outfit_list = {}
   for o,v in pairs(usable_outfits) do
      table.insert( outfit_list, o )
   end

   -- Optimization problem definition
   local ncols = 0
   local nrows = 0
   local ia = {}
   local ja = {}
   local ar = {}

   -- Figure out limits (both natural and artificial)
   local limits = {}
   local same_list = {}
   local same_limit = {}
   do
      local limit_list = {}
      for k,o in ipairs(outfit_list) do
         local n = o:nameRaw()
         -- Add limit if applicable
         local lim = o:limit()
         if lim then
            limit_list[lim] = true
         end
         -- See if we want to limit the particular outfit
         local t = o:slot()
         if params.max_same_weap and t=="Weapon" then
            table.insert( same_list, n )
            table.insert( same_limit, params.max_same_weap )
         elseif params.max_same_util and t=="Utility" then
            table.insert( same_list, n )
            table.insert( same_limit, params.max_same_util )
         elseif params.max_same_stru and t=="Structure" then
            table.insert( same_list, n )
            table.insert( same_limit, params.max_same_stru )
         end
      end
      -- Resort limits
      for k,v in pairs(limit_list) do
         table.insert( limits, k )
      end
   end

   -- Create outfit cache, it contains all sort of nice information like DPS and
   -- other stuff that can be used for our goodness function
   local outfit_cache = {}
   for k,out in ipairs(outfit_list) do
      -- Core stats
      --local oo    = out:shipstat(nil,true)
      local oo    = {}
      oo.name     = out:nameRaw()
      oo.outfit   = out
      oo.slot, oo.size = out:slot()
      oo.is_weap = (oo.slot=="Weapon")
      oo.is_util = (oo.slot=="Utility")
      oo.is_stru = (oo.slot=="Structure")
      local os = outfit_stats[oo.name]
      oo.stats    = os
      oo.dps, oo.disable, oo.eps, oo.range, oo.trackmin, oo.trackmax, oo.lockon, oo.iflockon, oo.seeker = out:weapstats( p )
      oo.trackmin = oo.trackmin or 0
      oo.trackmax = oo.trackmax or 0
      oo.lockon   = (oo.lockon or 0) + (oo.iflockon or 0)
      oo.cpu      = os.cpu
      oo.mass     = os.mass * ss.mass_mod
      oo.price    = os.price
      oo.limit    = os.limit
      if oo.limit then
         for i,l in ipairs(limits) do
            if l == oo.limit then
               oo.limitpos = i
               break
            end
         end
      end
      oo.type     = os.type
      oo.typebroad = os.typebroad
      oo.spec     = os.spec
      oo.isturret = oo.spec.isturret
      oo.penetration = oo.spec.penetration

      -- We correct ship stats here and convert them to "relative improvements"
      -- Movement
      oo.accel  = os.accel_mod * (os.accel + st.accel)  - st.accel
      oo.speed  = os.speed_mod * (os.speed  + st.speed) - st.speed
      oo.turn   = os.turn_mod  * (os.turn   + st.turn)  - st.turn
      -- Health
      oo.armour = os.armour_mod * (os.armour + st.armour) - st.armour
      oo.shield = os.shield_mod * (os.shield + st.shield) - st.shield
      oo.energy = os.energy_mod * (os.energy + st.energy) - st.energy
      oo.armour_regen = os.armour_regen_mod * (ss.armour_regen_mod * os.armour_regen + st.armour_regen) - os.armour_regen_malus - st.armour_regen
      oo.shield_regen = os.shield_regen_mod * (ss.shield_regen_mod * os.shield_regen + st.shield_regen) - os.shield_regen_malus - st.shield_regen
      oo.energy_regen = os.energy_regen_mod * (ss.energy_regen_mod * os.energy_regen + st.energy_regen) - os.energy_regen_malus - os.energy_loss - st.energy_regen
      oo.nebu_absorb = os.nebu_absorb
      -- Misc
      oo.cargo = os.cargo_mod * (os.cargo + ss.cargo) - ss.cargo

      -- Specific corrections
      if oo.type == "Fighter Bay" then
         -- Fighter bays don't have dps or anything, so we have to fake it
         oo.dps      = fbay_dps[oo.name]
         if not oo.dps then
            warn(string.format(_("Fighter bay '%s' does not have computed DPS!"), oo.name))
         end
         oo.disable  = 0
         oo.eps      = 0
         oo.range    = 10e3
         oo.penetration = 0.5
      elseif oo.type == "Afterburner" then
         -- We add it as movement, but weaken the effect a bit
         oo.accel    = oo.accel + 1.5*math.sqrt(oo.spec.accel * st.accel)
         oo.speed    = oo.speed + 1.5*math.sqrt(oo.spec.speed * st.speed)
      end

      -- Compute goodness
      oo.goodness = goodness_override[oo.name]
      if oo.goodness then
         oo.goodness = (params.prefer[oo.name] or 1) * (params.constant +  oo.goodness)
      else
         oo.goodness = params.goodness( oo, params )
      end

      -- Cache it all so we don't have to recompute
      outfit_cache[out] = oo
   end

   -- Figure out slots
   local slots = {}
   local slots_w, slots_u, slots_s = {}, {}, {}
   for k,v in ipairs(slots_base) do
      print( p:outfitSlot(k) )
      -- Must be empty
      if p:outfitSlot(k)==nil then
         local has_outfits = {}
         local outfitpos = {}
         for m,o in ipairs(outfit_list) do
            if ps:fitsSlot( k, o ) then
               table.insert( has_outfits, o )
               -- Check to see if it is in the similar list
               for spos,s in ipairs(same_list) do
                  if o==s then
                     outfitpos[ #has_outfits ] = spos
                     break
                  end
               end
            end
         end

         if #has_outfits > 0 then
            v.id = k
            v.outfits = has_outfits
            v.samepos = outfitpos
            table.insert( slots, v )

            -- Each slot adds a number of variables equivalent to the number of
            -- potential outfits, but only one constraint
            ncols = ncols + #v.outfits
            nrows = nrows + 1

            -- Sort by type to apply limits
            if v.type=="Weapon" then
               table.insert( slots_w, v )
            elseif v.type=="Utility" then
               table.insert( slots_u, v )
            elseif v.type=="Structure" then
               table.insert( slots_s, v )
            end
         end
      end
   end
   if ncols==0 then
      return -- Nothing to optimize
   end

   -- We have to add additional constraints (spaceworthy, limits)
   local sworthy = 3 -- Check CPU, energy regen, and mass
   -- Budget limit
   if params.budget then
      sworthy = sworthy + 1
   end
   -- For volatile systems we don't want ships to explode!
   local _nebu_dens, nebu_vol = system.cur():nebula()
   if nebu_vol > 0 then
      sworthy = sworthy + 1
   end
   -- Avoid same items and limits
   nrows = nrows + sworthy + #limits
   if #same_list > 0 then
      nrows = nrows + #same_list
   end
   -- Add max limits
   if params.max_weap then
      nrows = nrows+1
   end
   if params.max_util then
      nrows = nrows+1
   end
   if params.max_stru then
      nrows = nrows+1
   end
   local ntype_range = 0
   for k,v in pairs(params.type_range) do ntype_range = ntype_range+1 end
   nrows = nrows + ntype_range
   local lp = linopt.new( "equipopt", ncols, nrows, true )
   -- Add space worthy checks
   lp:set_row( 1, "CPU",          nil, st.cpu_max * ss.cpu_mod )
   local energygoal = math.max(params.min_energy_regen*st.energy_regen, params.min_energy_regen_abs)
   lp:set_row( 2, "energy_regen", nil, st.energy_regen - energygoal )
   local massgoal = math.max( params.max_mass * ss.engine_limit - st.mass, ss.engine_limit*params.min_mass_margin )
   if massgoal < 0 then
      warn(string.format(_("Impossible mass goal of %d set! Ignoring mass for pilot '%s'!"), massgoal, p:name()))
      massgoal = nil
   end
   lp:set_row( 3, "mass",      nil, massgoal )
   local rows = 3
   local budget_row
   if params.budget then
      rows = rows+1
      budget_row = rows
      lp:set_row( budget_row, "budget",    nil, params.budget )
   end
   local nebu_row
   local nebu_dmg
   if nebu_vol > 0 then
      rows = rows+1
      nebu_row = rows
      nebu_dmg = nebu_vol*(1-ss.nebu_absorb)
      lp:set_row( nebu_row, "shield_regen", nebu_dmg-st.shield_regen, nil )
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
   -- Add maximum amount of slots to use
   local r_weap, r_util, r_stru
   if params.max_weap then
      r_weap = r
      lp:set_row( r_weap, "max_weap", nil, params.max_weap )
      r = r+1
   end
   if params.max_util then
      r_util = r
      lp:set_row( r_util, "max_util", nil, params.max_util )
      r = r+1
   end
   if params.max_stru then
      r_stru = r
      lp:set_row( r_stru, "max_stru", nil, params.max_stru )
      r = r+1
   end
   -- Add outfit checks
   local c = 1
   for i,s in ipairs(slots) do
      for j,o in ipairs(s.outfits) do
         local stats = outfit_cache[o]
         local name = string.format("s%d-%s", i, stats.name)
         local slotmod = ((slots.size==stats.size) and 1) or params.mismatch
         local objf = (1+params.rnd*rnd.sigma()) * stats.goodness * slotmod -- contribution to objective function
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
            table.insert( ia, budget_row )
            table.insert( ja, c )
            table.insert( ar, stats.price )
         end
         -- Minimum shield regen is necessary
         if nebu_vol > 0 then
            table.insert( ia, nebu_row )
            table.insert( ja, c )
            table.insert( ar, stats.shield_regen + nebu_vol*stats.nebu_absorb )
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
         -- Maximum of slot type
         if params.max_weap then
            table.insert( ia, r_weap )
            table.insert( ja, c )
            table.insert( ar, 1 )
         end
         if params.max_util then
            table.insert( ia, r_util )
            table.insert( ja, c )
            table.insert( ar, 1 )
         end
         if params.max_stru then
            table.insert( ia, r_stru )
            table.insert( ja, c )
            table.insert( ar, 1 )
         end
         -- Check type range
         if ntype_range > 0 then
            local rn = params.type_range[ stats.name ]
            if rn then
               table.insert( ia, rn.id )
               table.insert( ja, c )
               table.insert( ar, 1 )
            end
            if stats.name ~= stats.type then
               rn = params.type_range[ stats.type ]
               if rn then
                  table.insert( ia, rn.id )
                  table.insert( ja, c )
                  table.insert( ar, 1 )
               end
            end
            if stats.type ~= stats.typebroad then
               rn = params.type_range[ stats.typebroad ]
               if rn then
                  table.insert( ia, rn.id )
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

   -- Load all the constraints
   lp:load_matrix( ia, ja, ar )

   -- Try to optimize
   local try = 0
   local emod = 1
   local mmod = 1
   local smod = 1
   local done
   local z, x, constraints
   repeat
      try = try + 1
      done = true
      -- All the magic is done here
      --lp:write_problem( "test.mps" )
      z, x, constraints = lp:solve( sparams )
      if not z then
         -- Try to relax constraints
         -- Mass constraint
         mmod = mmod * 2
         massgoal = mmod * params.max_mass * ss.engine_limit - st.mass
         lp:set_row( 3, "mass", nil, massgoal )
         -- Energy constraint
         energygoal = energygoal / 1.5
         lp:set_row( 2, "energy_regen", nil, st.energy_regen - emod*energygoal )

         -- Re-solve
         z, x, constraints = lp:solve( sparams )

         -- Likely nebula shield damage constraint if not resolved
         -- TODO this should probably just ignore the constraint and change it so that
         -- the pilot tries to optimize for maximum shield regen instead
         if not z and nebu_vol > 0 then
            smod = smod / 1.5
            lp:set_row( nebu_row, "shield_regen", smod*nebu_dmg-st.shield_regen, nil )

            -- Re-solve
            z, x, constraints = lp:solve( sparams )

            -- Check to see if that worked, and if not remove the constraint
            if not z then
               smod = 0
               lp:set_row( nebu_row, "shield_regen", nil, nil )

               -- Re-solve
               z, x, constraints = lp:solve( sparams )
            end
         end
      end

      if not z then
         -- Maybe should be error instead?
         warn(string.format(_("Failed to solve equipopt linear program for pilot '%s': %s"), p:name(), x))
         print_debug( p, st, ss, outfit_list, params, constraints, energygoal, emod, mmod, nebu_row, budget_row )
         return false
      end

      -- Interpret results
      c = 1
      for i,s in ipairs(slots) do
         for j,o in ipairs(s.outfits) do
            if x[c] == 1 then
               local q = p:outfitAdd( o, 1, true )
               if q < 1 then
                  warn(string.format(_("Unable to equip outfit '%s' on '%s'!"), o,  p:name()))
               end
            end
            c = c + 1
         end
      end

      -- Due to the approximation, sometimes they end up with not enough
      -- energy, we'll try again with larger energy constraints
      local stn = p:stats()
      if stn.energy_regen < energygoal then
         p:outfitRm( "all" )
         emod = emod * 1.5
         --print(string.format("Pilot %s: optimization attempt %d of %d: emod=%.3f", p:name(), try, 3, emod ))
         lp:set_row( 2, "energy_regen", nil, st.energy_regen - emod*energygoal )
         done = false
      end
   until done or try >= 5 -- attempts should be fairly fast since we just do optimization step
   if not done then
      warn(string.format(_("Failed to equip pilot '%s'!"), p:name()))
      print_debug( p, st, ss, outfit_list, params, constraints, energygoal, emod, mmod, nebu_row, budget_row )
      return false
   end

   -- Fill ammo
   p:fillAmmo()

   -- Set up useful outfits
   ai_setup.setup(p)

   -- Check
   if __debugging then
      local b, s = p:spaceworthy()
      if not b then
         warn(string.format(_("Pilot '%s' is not space worthy after equip script is run! Reason: %s"),p:name(),s))
         print_debug( p, st, ss, outfit_list, params, constraints, energygoal, emod, mmod, nebu_row, budget_row )
         return false
      end
   end
   return true
end

return optimize
