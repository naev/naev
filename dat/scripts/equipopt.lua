--[[

   Equips pilots based on mixed integer linear programming

--]]
local equipopt = {}

--[[
      Goodness functions to rank how good each outfits are
--]]
function equipopt.goodness_default( o )
   -- Constant value makes them prefer outfits rather than not
   local g = 1 - 0.00001*o.price
   -- Movement attributes
   g = g + 0.01*o.thrust + 0.1*o.speed + 0.1*o.turn
   -- Base attributes
   g = g - 0.01 * o.mass + 0.01 * o.energy
   -- Defensive attributes
   g = g + 0.01 * o.shield + 0.01 * o.armour
   -- Offensive attributes
   g = g + o.dps - 0.5 * o.eps
   return g
end

--[[
      Main equip script.
--]]
function equipopt.equip( p, cores, outfit_list, goodness )
   goodness = goodness or equipopt.goodness_default

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
      oo.dps, oo.eps = out:weapstats( p )
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

      -- We correct ship stats here and convert them to "relative improvements"
      oo.thrust = oo.thrust_mod * (oo.thrust + st.thrust) - st.thrust
      oo.speed  = oo.speed_mod  * (oo.speed  + st.speed)  - st.speed
      oo.turn   = oo.turn_mod   * (oo.turn   + st.turn)   - st.turn
      oo.armour = oo.armour_mod * (oo.armour + st.armour) - st.armour
      oo.shield = oo.shield_mod * (oo.shield + st.shield) - st.shield
      oo.energy = oo.energy_mod * (oo.energy + st.energy) - st.energy
      oo.armour_regen = oo.armour_regen_mod * (oo.armour_regen + st.armour_regen) - oo.armour_damage - st.armour_regen
      oo.shield_regen = oo.shield_regen_mod * (oo.shield_regen + st.shield_regen) - oo.shield_usage  - st.shield_regen
      oo.energy_regen = oo.energy_regen_mod * (oo.energy_regen + st.energy_regen) - oo.energy_usage  - oo.energy_loss - st.energy_regen

      -- Cache it all so we don't hae to recompute
      outfit_cache[v] = oo
   end

   -- Figure out slots
   local slots = {}
   local slots_base = ps:getSlots()
   for k,v in ipairs( slots_base ) do
      local has_outfits = {}

      for m,o in ipairs( outfit_list ) do
         if ps:fitsSlot( k, o ) then
            table.insert( has_outfits, o )
         end
      end

      if #has_outfits > 0 then
         v.id = k
         v.outfits = has_outfits
         print( string.format( "[%d] = %s, %s", k, v.type, v.size ) )
         for m, o in ipairs(v.outfits) do
            print( "   "..o )
         end
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
   lp = linopt.new( "test", ncols, nrows, true )
   -- Add space worthy checks
   lp:set_row( 1, "CPU",          nil, st.cpu )
   lp:set_row( 2, "energy_regen", nil, st.energy_regen )
   -- Add limit checks
   for i = 1,#limits do
      lp:set_row( sworthy+i, limits[i], nil, 1 )
   end
   -- Add outfit checks
   local c = 1
   local r = 1 + sworthy + #limits
   for i,s in ipairs(slots) do
      for j,o in ipairs(s.outfits) do
         local stats = outfit_cache[o]
         local name = string.format("s%d-o%d", i, j)
         local objf = goodness(stats) -- contribution to objective function
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
         c = c + 1
      end
      lp:set_row( r, string.format("s%d-sum", i), nil, 1 )
      r = r + 1
   end
   lp:load_matrix( ia, ja, ar )

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

   -- All the magic is done here
   z, x, c = lp:solve()
   for k,v in ipairs(x) do
      print(string.format("x%d: %d",k,v))
   end
   for k,v in ipairs(c) do
      print(string.format("c%d: %d",k,v))
   end

   -- Interpret results
   local c = 1
   for i,s in ipairs(slots) do
      for j,o in ipairs(s.outfits) do
         if x[c] == 1 then
            print( o )
         end
         c = c + 1
      end
   end
end

return equipopt
