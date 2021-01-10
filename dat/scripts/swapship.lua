require 'numstring'

local swapship = {}

--[[
-- @brief Tests to see if it is possible to swap ships for the player.
--]]
function swapship.test( template )
   local pp = player.pilot()

   local total_cargo = 0
   for k,v in ipairs( pp ) do
      total_cargo = total_cargo + v.q
   end

   -- See if we can swap over
   return total_cargo <= template:cargoFree()
end

--[[
-- @brief Swaps the player's ship.
--
--    @param template Should be a pilot template.
--    @return true on success, false otherwise
--]]
function swapship.swap( template )
   local pp = player.pilot()

   local total_cargo = 0
   local mission_cargo = 0
   for k,v in ipairs( pp:cargoList() ) do
      total_cargo = total_cargo + v.q
      -- Add mission cargo separately
      if v.m then
         mission_cargo = mission_cargo + v.q
      end
   end

   -- Impossible to move over, too much mission cargo
   if mission_cargo > template:cargoFree() then
      return false
   end

   -- Case not enough room to move stuff over
   if total_cargo > template:cargoFree() then
      -- Simulate cargo removal
      local cl = pp:cargoList()
      local space_needed = total_cargo-template:cargoFree()
      local removals = {}
      for k,v in ipairs( cl ) do
         if not v.m then
            v.p = commodity.get(v.nameRaw):price()
         end
      end
      while space_needed > 0 do
         -- Find cheapest
         local cn, cq, ck
         local cp = 1e10
         for k,v in pairs( cl ) do
            if not v.m then
               if v.p < cp then
                  ck = k
                  cn = v.nameRaw
                  cp = v.p
                  cq = v.q
               end
            end
         end
         -- Simulate removal
         cq = math.min( space_needed, cq )
         removals[cn] = cq
         cl[ck].q = cl[ck].q - cq
         if cl[ck].q <= 0 then
            cl[ck] = nil
         end
         space_needed = space_needed - cq
      end

      -- Format into a nice string all the removals
      local remove_str = ""
      for n,q in pairs(removals) do
         remove_str = remove_str .. string.format("\n %s %s", tonnestring_short(q), _(n))
      end

      -- Ask if the player is fine with removing the cargo
      if tk.yesno(
            _("Too much cargo for new ship"),
            _("You do not have enough space to move your cargo to the new ship. Get rid of the following to make room?")..remove_str) then
         -- Get rid of the cargo
         for n,q in pairs(removals) do
            pp:cargoRm( n, q )
         end
      else
         -- Player abandoned attempting to free cargo
         return false
      end
   end

   -- Create new ship
   player.swapShip( template:ship() )
   pp = player.pilot() -- Update struct to new pilot

   -- Copy equipment
   for k,v in ipairs( template:outfits() ) do
      pp:addOutfit( v, 1, true )
   end

   -- Delete pilot
   template:rm()

   return true
end

return swapship
