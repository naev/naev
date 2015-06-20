

--[[
-- @brief Handles the ship class by splitting it up into type/size.
--
-- Valid types are:
--  - civilian
--  - merchant
--  - military
--  - robotic
--
-- Valid sizes are:
--  - small
--  - medium
--  - large
--
--    @return Two parameters, first would be type, second would be size.
--]]
function equip_getShipBroad( class )

   -- Civilian
   if class == "Yacht" or class == "Luxury Yacht" then
      return "civilian", "small"
   elseif class == "Cruise Ship" then
      return "civilian", "medium"

   -- Merchant
   elseif class == "Courier" then
      return "merchant", "small"
   elseif class == "Freighter" or class == "Armoured Transport" then
      return "merchant", "medium"
   elseif class == "Bulk Carrier" then
      return "merchant", "large"

   -- Military
   elseif class == "Scout" or class == "Fighter" or class == "Bomber"  then
      return "military", "small"
   elseif class == "Corvette" or class == "Destroyer" then
      return "military", "medium"
   elseif class == "Cruiser" or class == "Carrier" then
      return "military", "large"

   -- Robotic
   elseif class == "Drone" then
      return "robotic", "small"
   elseif class == "Heavy Drone" then
      return "robotic", "small"
   elseif class == "Mothership" then
      return "robotic", "large"

   -- Unknown
   else
      print("Unknown ship of class '" .. class .. "'")
   end
end


--[[
-- @brief Fills the pilot slots with the outfits in the arrays.
--
--    @param p Pilot to fill outfit slots.
--    @param high High slots to use.
--    @param medium Medium slots to use.
--    @param low Low slots to use.
--    @param use_high Number of high slots to use (default max).
--    @param use_medium Number of medium slots to use (default max).
--    @param use_low Number of low slots to use (default max).
--]]
function equip_fillSlots( p, high, medium, low, use_high, use_medium, use_low )
   local nhigh, nmedium, nlow = p:ship():slots()
   -- Defaults
   use_high    = use_high or nhigh
   use_medium  = use_medium or nmedium
   use_low     = use_low or nlow
   -- Medium slots - for cpu/energy regen limits
   if #medium > 0 then
      local i = 0
      while i < use_medium do
         p:addOutfit( medium[ rnd.rnd(1,#medium) ] )
         i = i + 1
      end
   end
   -- Low slots
   if #low > 0 then
      local i = 0
      while i < use_low do
         p:addOutfit( low[ rnd.rnd(1,#low) ] )
         i = i + 1
      end
   end
   -- High slots
   if #high > 0 then
      local i = 0
      while i < use_high do
         p:addOutfit( high[ rnd.rnd(1,#high) ] )
         i = i + 1
      end
   end
end


--[[
-- @brief Adds an outfit.
--]]
function _equip_addOutfit( p, o, olist )
   if olist ~= nil then
      olist[ #olist+1 ] = o
      return 1
   else
      return p:addOutfit( o )
   end
end
--[[
-- @brief Ultimate equipment function, will set up a ship based on many parameters
--
--    @param p Pilot to equip.
--    @param scramble Use crazy assortment of primary/secondary weapons.
--    @param weapons A table of weapon tables, with the last key of each being the number of slots for that set.
--    @param medium List of medium outfits to use.
--    @param low List of low outfits to use.
--    @param reactor List of reactors to use.
--    @param use_medium Amount of slots to use for medium outfits (default nmedium).
--    @param use_low Amount of slots to use for low outfits (default nlow).
--    @param olist If not null, adds the outfits to olist instead of actually adding
--           them.
--]]
function equip_ship( p, scramble, weapons, medium, low,
   use_medium, use_low, olist )

   -- Get the ship
   if olist ~= nil then
      s = ship.get( p )
   else
      s = p:ship()
   end

   --[[
   --    Variables
   --]]
   local nhigh, nmedium, nlow = s:slots()
   local shipcpu = s:cpu()
   local shiptype, shipsize = equip_getShipBroad( s:class() )
   outfits = { }
   local i


   --[[
   --    Set up parameters that might be empty
   --]]
   use_medium     = use_medium or nmedium
   use_low        = use_low or nlow


   --[[
   --    Set up weapons
   --]]
   equip_parseWeapons(p, scramble, s)

   -- Add high slots
   for k,v in ipairs(outfits) do
      _equip_addOutfit( p, v, olist )
   end


   --[[
   --    Medium and low slots
   --]]
   outfits  = { }
   -- Medium slots
   i        = 0
   while i < use_medium do
      outfits[ #outfits+1 ] = medium[ rnd.rnd(1,#medium) ]
      i = i + 1
   end
   -- Low slots
   i        = 0
   while i < use_low do
      outfits[ #outfits+1 ] = low[ rnd.rnd(1,#low) ]
      i = i + 1
   end
   -- Add slots
   for k,v in ipairs(outfits) do
      _equip_addOutfit( p, v, olist )
   end
end


-- Cache to avoid repeat lookups.
scache = {}
ocache = {}


-- TODO: Expose slots via the Lua API
_slotSizes = { Small = 1, Medium = 2, Large = 3 }


-- Caches the sizes of a ship's weapon slots.
function cache_ship( ship, name )
   if scache[name] then
      return
   end

   scache[name] = {}
   for k,v in ipairs( ship:getSlots() ) do
      if v['type'] == "weapon" then
         table.insert(scache[name], _slotSizes[ v['size'] ])
      end
   end
end


-- Caches outfit sizes.
function cache_outfit( name )
   if ocache[name] then
      return
   end

   local outfit = outfit.get(name)
   local _, size = outfit:slot()
   ocache[name] = _slotSizes[size]
end


function equip_findOutfit( shipname, slot, outfits )
   local o
   for j=1, #outfits-1, 1 do
      o = outfits[j]
      cache_outfit(o)

      if ocache[o] <= scache[shipname][slot] then
         return v
      end
   end

   warn(string.format("Could not find an outfit fitting weapon slot %d for '%s'",
         slot, shipname ))
end


--[[
-- @brief Parses a table of weapon tables with reasonable flexibility.
--
--    @param Pilot to add weapons to
--    @param Whether or not to randomly select outfits.
--    @param Ship type (optional)
--]]
function equip_parseWeapons( p, scramble, ship )
   local pname, name, abort

   if not ship then
      ship = p:ship()
   end

   if p and type(p) == "userdata" then
      pname = p:name()
   else
      pname = ship:name()
   end

   name = ship:name()

   -- Cache ship's weapon slot sizes.
   cache_ship(ship, name)

   for ak,av in ipairs(weapons) do
      local i, o, shuffled

      i = 0
      o = av[ rnd.rnd(1, #av-1) ]
      cache_outfit(o)

      if #outfits + av[#av] > #scache[name] then
         av[#av] = #scache[name] - #outfits
         abort = true
      end

      if not scramble then
         while i < av[#av] do
            if o and (ocache[o] > scache[name][#outfits + 1]) then
               if not shuffled then
                  shuffled = _shuffle( av, #av-1 )
               end

               o = equip_findOutfit( name, #outfits + 1, shuffled )
            end

            if o then
               outfits[ #outfits+1 ] = o
            end
            i = i + 1
         end
      else
         while i < av[#av] do
            o = av[ rnd.rnd(1, #av-1) ]
            cache_outfit(o)

            if ocache[o] > scache[name][#outfits + 1] then
               o = equip_findOutfit( name, #outfits + 1, _shuffle( av, #av-1 ) )
            end

            if o then
               outfits[ #outfits+1 ] = o
            end
            i = i + 1
         end
      end

      if abort then
         warn( string.format("Attempting to equip more than %d weapons on '%s', aborting.",
               #scache[name], pname) )
         return
      end
   end
end


function _shuffle( t, max )
   local n, k

   n = max
   while n > 1 do
      k = math.random(n)
      n = n - 1
      t[n], t[k] = t[k], t[n]
   end

   return t
end


function icmb( t1, t2 )
   t = {}
   for _,v in ipairs(t1) do
      t[ #t+1 ] = v
   end
   for _,v in ipairs(t2) do
      t[ #t+1 ] = v
   end
   return t
end


function addWeapons( new, limit )
   if not weapons then
      weapons = {}
   end
   table.insert(new, limit)
   return table.insert(weapons, new)
end
