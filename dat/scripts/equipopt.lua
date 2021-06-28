--[[

   Equips pilots based on mixed integer linear programming

--]]
local equipopt = {}

--[[
      Goodness functions to rank how good each outfits are
--]]
function equipopt.goodness_default( o, p )
   -- Base attributes
   base = -0.000001*o.price - 0.003*o.mass + 0.1*o.cargo - 0.5*o.cargo_inertia + 0.003*o.fuel
   -- Movement attributes
   move = 0.05*o.thrust + 0.05*o.speed + 0.1*o.turn
   -- Health attributes
   health = 0.005*o.shield + 0.005*o.armour + 0.005*o.energy + o.absorb + 0.1*o.shield_regen + 0.1*o.armour_regen + 0.1*o.energy_regen
   -- Weapon attributes
   if o.dps and o.dps > 0 then
      local trackmod = math.min( 1, math.max( 0, (p.tracktarget-o.trackmin)/(o.trackmax-o.trackmin)) )
      weap = 0.05*o.dps - 0.02*o.eps + 0.01*o.range
      weap = weap * (trackmod+0.1)
      if p.prefertur and not o.isturret then
         weap = weap * 0.75
      end
   else
      weap = 0
   end
   -- Ewarfare attributes
   ew = 3*(o.ew_detect-1) + 3*(o.ew_hide-1)
   --print(string.format("%s: base = %.3f, move = %.3f, health = %.3f, weap = %.3f, ew = %.3f", o.outfit:name(), base, move, health, weap, ew))
   if 1+base+move+health+weap+ew < 0 then
      print(string.format("Outfit %s has negative goodness: %.3f", o.outfit:name(), 1+base+move+health+weap+ew))
      print(string.format("%s: base = %.3f, move = %.3f, health = %.3f, weap = %.3f, ew = %.3f", o.outfit:name(), base, move, health, weap, ew))
   end
   -- Constant value makes them prefer outfits rather than not
   return 1 + base + move + health + weap + ew
end

equipopt.params_default = {
   goodness = equipopt.goodness_default,

   tracktarget = 10000,
   prefertur = true,
   maxsame = 3,
}

--[[
      Main equip script.
--]]
function equipopt.equip( p, cores, outfit_list, params )
   params = params or equipopt.params_default

   -- Naked ship
   p:rmOutfit( "all" )
   p:rmOutfit( "cores" )
   -- Put cores
   for k,v in ipairs( cores ) do
      p:addOutfit( v, 1, true )
   end

   -- Global ship stuff
   local ps = p:ship()
   local ss = p:shipstat( nil, true ) -- Should include cores!!
   local st = p:stats() -- also include cores

   -- Optimization problem definition
   local ncols = 0
   local nrows = 0
   local ia = {}
   local ja = {}
   local ar = {}

   -- Figure out limits
   local limit_list = {}
   for k,v in ipairs(outfit_list) do
      local oo = outfit.get(v)
      -- Add limit if applicable
      local lim = oo:limit()
      if lim then
         limit_list[lim] = true
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
      oo.dps, oo.eps, oo.range, oo.trackmin, oo.trackmax, oo.lockon = out:weapstats( p )
      oo.trackmin = oo.trackmin or 0
      oo.trackmax = oo.trackmax or 0
      oo.lockon   = oo.lockon or 0
      oo.cpu      = out:cpu() * ss.cpu_mod
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
      if oo.type == "Bolt Turret" or oo.type == "Beam Turret" or oo.type == "Turret Launcher" then
         oo.isturret = true
      end
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

      -- Compute goodness
      oo.goodness = params.goodness( oo, params )

      -- Cache it all so we don't hae to recompute
      outfit_cache[v] = oo
   end

   -- Figure out slots
   local slots = {}
   local slots_base = ps:getSlots()
   for k,v in ipairs( slots_base ) do
      local has_outfits = {}
      local outfitpos = {}
      for m,o in ipairs( outfit_list ) do
         if ps:fitsSlot( k, o ) then
            table.insert( has_outfits, o )
            table.insert( outfitpos, m )
         end
      end

      if #has_outfits > 0 then
         v.id = k
         v.outfits = has_outfits
         v.outfitpos = outfitpos
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
   local sworthy = 2 -- Check CPU and Energy regen
   nrows = nrows + sworthy + #limits
   if params.maxsame then
      nrows = nrows + #outfit_list
   end
   lp = linopt.new( "test", ncols, nrows, true )
   -- Add space worthy checks
   lp:set_row( 1, "CPU",          nil, st.cpu )
   lp:set_row( 2, "energy_regen", nil, st.energy_regen )
   -- Add limit checks
   for i,l in ipairs(limits) do
      lp:set_row( sworthy+i, l, nil, 1 )
   end
   local nsame = 0
   if params.maxsame then
      for i,o in ipairs(outfit_list) do
         lp:set_row( sworthy+#limits+i, o, nil, params.maxsame )
      end
      nsame = #outfit_list
   end
   -- Add outfit checks
   local c = 1
   local r = 1 + sworthy + #limits + nsame
   for i,s in ipairs(slots) do
      for j,o in ipairs(s.outfits) do
         local stats = outfit_cache[o]
         local name = string.format("s%d-o%d", i, j)
         local objf = stats.goodness -- contribution to objective function
         lp:set_col( c, name, objf, "binary" ) -- constraints set automatically
         -- CPU constraint
         table.insert( ia, 1 )
         table.insert( ja, c )
         table.insert( ar, -stats.cpu )
         -- Energy constraint
         table.insert( ia, 2 )
         table.insert( ja, c )
         table.insert( ar, -stats.energy_regen )
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
         if params.maxsame then
            table.insert( ia, sworthy + #limits + s.outfitpos[j] )
            table.insert( ja, c )
            table.insert( ar, 1 )
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
   --[[
   for k,v in ipairs(x) do
      print(string.format("x%d: %d",k,v))
   end
   for k,v in ipairs(c) do
      print(string.format("c%d: %d",k,v))
   end
   --]]

   -- Interpret results
   print("Final Equipment:")
   local c = 1
   for i,s in ipairs(slots) do
      for j,o in ipairs(s.outfits) do
         if x[c] == 1 then
            print( "   "..o )
         end
         c = c + 1
      end
   end
end

return equipopt
