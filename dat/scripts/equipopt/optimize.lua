--[[--
   Equips pilots based on mixed integer linear programming.

   @module equipopt.optimize
--]]
local optimize = {}
local eparams = require 'equipopt.params'
local bioship = require 'bioship'
local ai_setup = require "ai.core.setup"
local fmt = require "format"
local lf = require "love.filesystem"
local function choose_one( t ) return t[ rnd.rnd(1,#t) ] end

local fighterbays_data = {}
for k,v in ipairs(lf.getDirectoryItems("scripts/equipopt/fighterbays")) do
   local fb = require( "equipopt.fighterbays."..string.gsub(v,".lua","") )
   fb.priority = fb.priority or 5
   table.insert( fighterbays_data, fb )
end
table.sort( fighterbays_data, function (a, b)
   -- Lower priority ru later to overwrite
   return a.priority > b.priority
end )
local fighterbays = {}
for k,v in ipairs(fighterbays_data) do
   fighterbays[ v.ship:nameRaw() ] = v
end

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
   ["Nexus Concealment Coating"] = 3,
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
   ["Hyperbolic Blink Engine"] = 2,
   ["S&K Heavy Plasma Drill"] = 1,
   ["S&K Plasma Drill"] = 1,
}

-- Special weights
local goodness_special = {
   ["Black Cat Doll"] = 0.25, -- Only active when stealthing
   ["TeraCom Mace Launcher"] = 0.5,
   ["Unicorp Caesar IV Launcher"] = 0.6, -- high damage but slow with bad tracking
   ["Enygma Systems Huntsman Launcher"] = 0.6,
   ["Enygma Systems Spearhead Launcher"] = 0.6, -- high damage but shield only
   ["TeraCom Vengeance Launcher"] = 0.6,
   ["TeraCom Medusa Launcher"] = 0.6,           -- really high disable
   ["Droid Repair Crew"] = 0.35, -- Only work until 50%
   ["Electron Burst Cannon"] = 0.7, -- Shieldbreaker damage
   ["Particle Beam"] = 0.8,
   ["EMP Grenade Launcher"] = 0.36,

   -- Point defence works against missiles so it's extra useful
   ["Ratchet Point Defence"] = 1.52,
   ["Dvaered Flare Battery"] = 1.46,
   ["Guardian Interception System"] = 1.21,
   ["Guardian Overseer System"] = 1.15,
   ["ZIBS-16"] = 2.77,
   ["ZIBS-32"] = 2.71,
   ["Spittle Tubuloid Cluster"] = 1.9,
   -- Plasma do a lot of damage over time
   ["Plasma Blaster MK1"] = 1 / 0.63,
   ["Plasma Blaster MK2"] = 1 / 0.63,
   ["Plasma Cannon"] = 1 / 0.63,
   ["Plasma Cluster Cannon"] = 1 / 0.63,
   ["Plasma Turret MK1"] = 1 / 0.70,
   ["Plasma Turret MK2"] = 1 / 0.70,
   -- Razor/Disruptors do disable over time (and energy drain!)
   ["Razor Artillery S1"] = 1 / 0.80,
   ["Razor Artillery S2"] = 1 / 0.80,
   ["Razor Artillery S3"] = 1 / 0.80,
   ["Razor Battery S3"] = 1 / 0.80,
   ["Disruptor Artillery S1"] = 1 / 0.80,
   ["Disruptor Artillery S2"] = 1 / 0.80,
   ["Disruptor Battery S3"] = 1 / 0.80,
}


--[[
      Completely custom ship builds: they do not use optimization
--]]
local special_ships = {}
special_ships["Drone"] = function( p )
   local sys = outfit.get( "Milspec Orion 2301 Core System" )
   p:outfitAddSlot( sys, "systems", true )
   local eng = outfit.get( "Nexus Dart 160 Engine" )
   p:outfitAddSlot( eng, "engines", true )
   local hul = outfit.get( choose_one{"Nexus Shadow Weave", "S&K Skirmish Plating"} )
   p:outfitAddSlot( hul, "hull", true )
   for k,o in ipairs{
      "Neutron Disruptor",
      "Neutron Disruptor",
      "Neutron Disruptor",
   } do
      p:outfitAdd( o, 1, true )
   end
end
special_ships["Heavy Drone"] = function( p )
   local sys = outfit.get( "Milspec Thalos 2202 Core System" )
   p:outfitAddSlot( sys, "systems", true )
   p:outfitAddSlot( sys, "systems_secondary", true )
   local eng = outfit.get( "Nexus Dart 160 Engine" )
   p:outfitAddSlot( eng, "engines", true )
   p:outfitAddSlot( eng, "engines_secondary", true )
   local hul = outfit.get( choose_one{"Nexus Shadow Weave", "S&K Skirmish Plating"} )
   p:outfitAddSlot( hul, "hull", true )
   hul = outfit.get( "S&K Skirmish Plating" )
   p:outfitAddSlot( hul, "hull_secondary", true )
   for k,o in ipairs{
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
   local base = p.cargo*(0.5*math.pow(o.cargo,0.3) + 0.1*(1-os.cargo_inertia)) + p.fuel*0.2*o.fuel
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
      if o.ispd then
         weap = weap * p.pointdefence
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


local function print_debug( lp, p, st, ss, outfit_list, params, constraints, energygoal, emod, mmod, nebu_row, _budget_row )
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
   print(string.format(_("CPU: %d / %d [x=%d < %d]"), stn.cpu, stn.cpu_max, constraints[1] or 0, st.cpu_max ))
   print(string.format(_("Energy Regen: %.3f [x=%.3f > %.3f (%.1f)]"), stn.energy_regen, constraints[2] or 0, st.energy_regen - emod*energygoal, emod))
   print(string.format(_("Mass: %.3f / %.3f [x=3f < %.3f (%.1f)]"), st.mass, ss.engine_limit, constraints[3] or 0, mmod * params.max_mass * ss.engine_limit - st.mass, mmod ))
   if nebu_row then
      local _nebu_dens, nebu_vol = system.cur():nebula()
      print(string.format(_("Shield Regen: %.3f [x=%.3f > %.3f (%.1f)]"), stn.shield_regen, constraints[nebu_row] or 0, nebu_vol*(1-ss.nebu_absorb)-st.shield_regen, 1))
   end
   if __debugging then
      local outfile = fmt.f( "logs/linopt-{date}-{ship}{shipid}.mps", {date=naev.date("%Y-%m-%d_%H:%M:%S"), ship=p:name(), shipid=p:id()})
      lp:write_problem( outfile )
      print(fmt.f(_("Wrote optimization problem to '{path}'!"),{path=outfile}))
   end
end

local function compute_goodness( outfit_list, p, st, ss, params, limits )
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
      oo.cpu      = os.cpu - st.cpu_max * os.cpu_mod + st.cpu_max - os.cpu_max
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
      oo.ispd     = out:pointdefence()

      -- We correct ship stats here and convert them to "relative improvements"
      -- Movement
      oo.accel  = os.accel_mod * (os.accel + st.accel) - st.accel
      oo.speed  = os.speed_mod * (os.speed + st.speed) - st.speed
      oo.turn   = os.turn_mod  * (os.turn  + st.turn)  - st.turn
      -- Health
      oo.armour = os.armour_mod * (os.armour + st.armour) - st.armour
      oo.shield = os.shield_mod * (os.shield + st.shield) - st.shield
      oo.energy = os.energy_mod * (os.energy + st.energy) - st.energy
      oo.armour_regen = os.armour_regen_mod * (ss.armour_regen_mod * os.armour_regen + st.armour_regen) - os.armour_regen_malus - st.armour_regen
      oo.shield_regen = os.shield_regen_mod * (ss.shield_regen_mod * os.shield_regen + st.shield_regen) - os.shield_regen_malus - st.shield_regen
      oo.energy_regen = os.energy_regen_mod * (ss.energy_regen_mod * os.energy_regen + st.energy_regen) - os.energy_regen_malus - st.energy_regen
      oo.nebu_absorb = os.nebu_absorb
      -- Misc
      oo.cargo = os.cargo_mod * (os.cargo + ss.cargo) - ss.cargo
      oo.fuel  = (os.fuel_mod * (os.fuel + ss.fuel) - ss.fuel) / st.fuel_consumption

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
   return outfit_cache
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
   local rndness = params.rnd

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

   -- Special case fighters, we want consistent equipment when possible
   if p:flags("carried") then
      local fb = fighterbays[ ps:nameRaw() ]
      if fb ~= nil then
         if fb.equip( p ) then
            ai_setup.setup(p)
            return
         end
      end
      rndness = 0
   end

   -- Special case bioships
   if pt.bioship and not p:shipvarPeek("bioship_init") then
      local stage = bioship.maxstage( p )
      bioship.simulate( p, rnd.rnd(1,stage) )
   end

   -- Handle cores
   if cores and not pt.nocores then
      for k,v in pairs( cores ) do
         -- Maybe remove this debugging check sometime
         if type(k) ~= "string" then
            warn(fmt.f(_("Invalid core table with non-string key '{key}' and value '{value}'"),
               {key=k, value=v}))
         else
            p:outfitRmSlot( k )
            if not p:outfitAddSlot( v, k, true ) then
               warn(fmt.f(_("Unable to equip core '{outfit}' on '{pilot}'!"),
                  {outfit=v, pilot=p}))
            end
         end
      end
   end

   -- Store base outfits, including new cores and such
   local outfits_base = p:outfits()

   -- Global ship stuff
   local ss = p:shipstat( nil, true ) -- Should include cores!!
   local st = p:stats() -- also include cores
   st.cpu = st.cpu_max / ss.cpu_mod -- Base value to modulate

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
   local outfit_cache = compute_goodness( outfit_list, p, st, ss, params, limits )

   -- Figure out slots
   local slots = {}
   local slots_w, slots_u, slots_s = {}, {}, {}
   for k,v in ipairs(slots_base) do
      -- Must be empty and not locked
      if p:outfitSlot(k)==nil and not v.locked then
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
      p:fillAmmo()
      ai_setup.setup(p)
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
   lp:set_row( 1, "CPU",          nil, st.cpu_max ) -- Don't multiply by modifiers here or they get affected "twice"
   local energygoal = math.max((1-params.min_energy_regen)*st.energy_regen, params.min_energy_regen_abs)
   lp:set_row( 2, "energy_regen", energygoal - st.energy_regen )
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
         local objf = (1+rndness*rnd.sigma()) * stats.goodness * slotmod -- contribution to objective function
         lp:set_col( c, name, objf, "binary" ) -- constraints set automatically
         -- CPU constraint
         table.insert( ia, 1 )
         table.insert( ja, c )
         table.insert( ar, -stats.cpu )
         -- Energy constraint
         table.insert( ia, 2 )
         table.insert( ja, c )
         table.insert( ar, stats.energy_regen-params.eps_weight*(stats.eps or 0) )
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
         if params.max_weap and stats.is_weap then
            table.insert( ia, r_weap )
            table.insert( ja, c )
            table.insert( ar, 1 )
         end
         if params.max_util and stats.is_util then
            table.insert( ia, r_util )
            table.insert( ja, c )
            table.insert( ar, 1 )
         end
         if params.max_stru and stats.is_stru then
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
   local min_energy = params.min_energy_regen_abs - st.energy_regen
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
         -- Energy constraint, ensure doesn't go over base
         energygoal = energygoal / 1.5
         lp:set_row( 2, "energy_regen", math.max( min_energy, emod*energygoal - st.energy_regen ))

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
         if try >= 5 then
            -- Maybe should be error instead?
            warn(string.format(_("Failed to solve equipopt linear program for pilot '%s': %s"), p:name(), x))
            print_debug( lp, p, st, ss, outfit_list, params, constraints, energygoal, emod, mmod, nebu_row, budget_row )
         end

      -- Interpret results by equipping
      else
         c = 1
         for i,s in ipairs(slots) do
            for j,o in ipairs(s.outfits) do
               if x[c] == 1 then
                  if not  p:outfitAddSlot( o, s.id, true ) then
                     warn(string.format(_("Unable to equip outfit '%s' on '%s'!"), o,  p:name()))
                  end
               end
               c = c + 1
            end
         end
      end

      -- Due to the approximation, sometimes they end up with not enough
      -- energy, we'll try again with more relaxed energy constraints
      local stn = p:stats()
      if not z or (stn.energy_regen < energygoal and try < 5 and (st.energy_regen - emod*energygoal) > min_energy) then
         p:outfitsEquip( outfits_base ) -- Should restore initial outfits
         emod = emod * 1.5
         print(string.format("Pilot %s: optimization attempt %d of %d: emod=%.3f", p:name(), try, 3, emod ))
         lp:set_row( 2, "energy_regen", math.max( min_energy, st.energy_regen - emod*energygoal ))
         done = false
      end
   until done or try >= 5 -- attempts should be fairly fast since we just do optimization step
   if not done then
      warn(string.format(_("Failed to equip pilot '%s'!"), p:name()))
      print_debug( lp, p, st, ss, outfit_list, params, constraints, energygoal, emod, mmod, nebu_row, budget_row )
      return false
   end

   -- Fill ammo and heal
   p:fillAmmo()
   p:setHealth( 100, 100, 0 ) -- New cores set health to 0 otherwise

   -- Set up useful outfits
   ai_setup.setup(p)

   -- Check
   if __debugging then
      local b, s = p:spaceworthy()
      if not b then
         warn(string.format(_("Pilot '%s' is not space worthy after equip script is run! Reason: %s"),p:name(),s))
         print_debug( lp, p, st, ss, outfit_list, params, constraints, energygoal, emod, mmod, nebu_row, budget_row )
         return false
      end
   end
   return true
end

function optimize.debug_goodness( p, params, outfits )
   params = params or eparams.default()
   outfits = outfits or outfit.getAll()
   params.goodness = params.goodness or optimize.goodness_default

   p = p or player.pilot()
   local st = p:stats()
   local ss = p:shipstat( nil, true ) -- Should include cores!!

   local ocache = compute_goodness( outfits, p, st, ss, params, {} )
   local oo = {}
   for k,v in pairs(ocache) do
      -- Skip core outfits
      local t = v.outfit:tags()
      if not t.core and not t.noplayer then
         table.insert( oo, v )
      end
   end
   table.sort( oo, function ( a, b )
      return a.goodness > b.goodness
   end )
   for k,v in ipairs(oo) do
      print( v.outfit, v.goodness )
   end
end

return optimize
