

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
      return "robotic", "medium"
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
      return true
   else
      return p:addOutfit( o )
   end
end
--[[
-- @brief Ultimate equipment function, will set up a ship based on many parameters
--
--    @param p Pilot to equip.
--    @param scramble Use crazy assortment of primary/secondary weapons.
--    @param primary List of primary weapons to use.
--    @param secondary List of secondary weapons to use.
--    @param medium List of medium outfits to use.
--    @param low List of low outfits to use.
--    @param apu List of APU to use.
--    @param reactor List of reactors to use.
--    @param use_primary Amount of slots to use for primary (default nhigh-1).
--    @param use_secondary Amount of slots to use for secondary (default 1).
--    @param use_medium Amount of slots to use for medium outfits (default nmedium).
--    @param use_low Amount of slots to use for low outfits (default nlow).
--    @param olist If not null, adds the outfits to olist instead of actually adding
--           them.
--]]
function equip_ship( p, scramble, primary, secondary, medium, low, apu,
   use_primary, use_secondary, use_medium, use_low, olist )

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
   local outfits = { }
   local i


   --[[
   --    Set up parameters that might be empty
   --]]
   use_primary    = use_primary or nhigh-1
   use_secondary  = use_secondary or 1
   use_medium     = use_medium or nmedium
   use_low        = use_low or nlow


   --[[
   --    Set up weapons
   --]]
   -- Check uniformity
   local po, so
   if not scramble then
      po = primary[ rnd.rnd(1,#primary) ]
      so = secondary[ rnd.rnd(1,#secondary) ]
   end
   -- Primary
   i = 0
   local o = primary[ rnd.rnd(1,#primary) ]
   while i < use_primary do
      outfits[ #outfits+1 ] = po or primary[ rnd.rnd(1,#primary) ]
      i = i + 1
   end
   -- Secondary
   i = 0
   o = secondary[ rnd.rnd(1,#secondary) ]
   while i < use_secondary do
      outfits[ #outfits+1 ] = so or secondary[ rnd.rnd(1,#secondary) ]
      i = i + 1
   end
   -- Check CPU if we can add APU
   if apu ~= nil and #apu > 0 then
      local cpu_usage = 0
      for k,v in ipairs( outfits ) do
         cpu_usage = cpu_usage + s.outfitCPU( v )
      end
      local added = true
      while added and use_medium > 0 and cpu_usage > shipcpu do -- Need to add APU
         local o_apu
         o_apu       = apu[ rnd.rnd(1,#apu) ]
         added       = _equip_addOutfit( p, o_apu, olist )
         shipcpu     = shipcpu + s.outfitCPU( o_apu )
         use_medium  = use_medium - 1 -- Discount from available medium slots
      end
   end
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

