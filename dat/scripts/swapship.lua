--[[--
Utility to swap the player's ship.

@module swapship
--]]
local fmt = require "format"

local swapship = {}

--[[--
   Tests to see if it is possible to swap ships for the player.

   @tparam Pilot template Template pilot to try to swap ships with the player.
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

--[[--
   Swaps the player's ship. Performs checks to make sure things don't break. Use in place of player.shipSwap() unless you really know what you're doing.

   Can fail if the player can't copy all the mission cargo over.

   @tparam Pilot template Template pilot to swap with the player. Note that the template pilot gets removed on success.
   @tparam string acquired Description of how the new ship was acquired.
   @treturn boolean true on success, false otherwise
--]]
function swapship.swap( template, acquired )
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
            v.p = commodity.get(v.name):price()
         end
      end
      while space_needed > 0 do
         -- Find cheapest
         local cn, cq, ck
         local cp = math.huge
         for k,v in pairs( cl ) do
            if not v.m then
               if v.p < cp then
                  ck = k
                  cn = v.name
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
         remove_str = remove_str .. string.format("\n %s %s", fmt.tonnes_short(q), _(n))
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

   -- Create new ship and swap to it
   local newship = player.shipAdd( template:ship(), nil, acquired )
   player.shipSwap( newship )
   pp = player.pilot() -- Update struct to new pilot

   -- Start with an empty ship
   pp:outfitRm( "all" )
   pp:outfitRm( "cores" )

   -- Copy equipment
   for k,v in ipairs( template:outfitsList() ) do
      pp = player.pilot()
      pp:outfitAdd( v, 1, false )
   end

   -- Delete pilot
   template:rm()

   return true
end

return swapship
