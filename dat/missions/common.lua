--[[

      Common Lua Mission framework

--]]


-- The common function table.
common = {}


--[[
         COMBAT TIER STUFF
--]]
--[[
   @brief Calculates the mission combat tier.

   Calculations are done with ally and enemy values at the instance where they
    are highest. To calculate the enemy/ally ratings you use the following
    table:

   - Lone Fighter/Bomber: +1
   - Fighter/Bomber Squadron: +3
   - Corvette: +4
   - Destroyer: +6
   - Cruiser: +10
   - Carrier: +12
   - Small Fleet: +10 (equivalent to small fleet in fleet.xml)
   - Medium Fleet: +20 (equivalent to medium fleet in fleet.xml)
   - Large Fleet: +30 (equivalent to heavy fleet in fleet.xml) 

      @param enemy_value Peak enemy value.
      @param ally_value Ally value for peak enemy value.
      @param must_destroy Whether or not player must destroy the enemies (default: false)
      @param bkg_combat Whether or not combat is in the background without player as target (default: false)
   @luareturn The combat tier level (also stores it internally).
--]]
common.calcMisnCombatTier = function( enemy_value, ally_value, must_destroy, bkg_combat )

   -- Set defaults
   must_destroy = must_destroy or true
   bkg_combat   = bkg_combat or false

   -- Calculate modifiers
   local mod = 0
   if not must_destroy then
      mod = mod * 0.5
   end
   if bkg_combat then
      mod = mod * 0.75
   end

   -- Calculate rating value
   local rating = mod * (enemy_value - ally_value/3)

   -- Get Tier from rating
   local tier
   if rating <= 0 then
      tier = 0
   elseif rating <= 2 then
      tier = 1
   elseif rating <= 4 then
      tier = 2
   elseif rating <= 8 then
      tier = 3
   elseif rating <= 12 then
      tier = 4
   elseif rating <= 17 then
      tier = 5
   elseif rating <= 22 then
      tier = 6
   else
      tier = 7
   end

   -- Set tier
   common.setCombatTier( tier )

   -- Return tier
   return tier
end


--[[
   @brief Calculates the player combat tier

      @return The player's equivalent combat tier.
--]]
common.calcPlayerCombatTier = function ()
   local crating = player.getRating()
   local tier

   -- Calculate based off of combat rating
   if crating <= 5 then
      tier = 0
   elseif crating <= 10 then
      tier = 1
   elseif crating <= 20 then
      tier = 2
   elseif crating <= 40 then
      tier = 3
   elseif crating <= 80 then
      tier = 4
   elseif crating <= 160 then
      tier = 5
   elseif creating <= 320 then
      tier = 6
   else
      tier = 7
   end

   return tier
end


--[[
   @brief Sets the combat tier

      @param tier Tier to set mission combat tier to.
--]]
common.setTierCombat = function( tier )
   common["__tierCombat"] = tier
end


--[[
   @brief Gets the combat tier

      @return The combat tier of the mission.
--]]
common.getTierCombat = function ()
   return common["__tierCombat"]
end


--[[
   @brief Checks to see if the player meets a minum tier

      @param tier Tier to check for (if nil tries to use the mission tier)
--]]
common.checkTierCombat = function( tier )
   tier = tier or common.getTierCombat()

   -- No tier set so it always matches
   if tier == nil then
      return true
   end

   -- Check against mission tier
   return common.calcPlayerCombatTier() >= tier
end



--[[
      REWARD STUFF
--]]
function _calcTierReward( tier )
   local low, high
   if tier <= 0 then
      low  = 0
      high = 0
   elseif tier == 1 then
      low  = 0
      high = 50
   elseif tier == 2 then
      low  = 50
      high = 100
   elseif tier == 3 then
      low  = 100
      high = 150
   elseif tier == 4 then
      low  = 150
      high = 200
   elseif tier == 5 then
      low  = 200
      high = 250
   elseif tier == 6 then
      low  = 250
      high = 275
   else
      low  = 275
      high = 300
   end
   return low, high
end
--[[
   @brief Calculates the monetary reward based on the tiers of the mission

   @usage low, high = common.calcMonetaryReward()

      @return The amount of money the player should be paid as an interval.
--]]
common.calcMonetaryReward = function ()
   local low  = 0
   local high = 0
   local l,h
   local tiers = {}

   -- Get tiers
   tiers[ #tiers+1 ] = common.getTierCombat()

   -- Calculate money
   for k,v in ipairs(tiers) do
      l, h = _calcTierReward( v )
      low  = low + l
      high = high + h
   end

   -- Return total amount
   return low, high
end



