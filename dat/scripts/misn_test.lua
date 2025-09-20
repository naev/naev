--[[

   Simple library to handle common conditional expressions, mainly focused on
   mission computer missions.

--]]
local misn_test = {}

--[[--
   @brief Test for cargo missions.
--]]
function misn_test.cargo( notcomputer )
   -- Has to support normal factions
   local sc = spob.cur()
   if sc:tags().restricted then
      return false -- Restricted places don't offer these missions currently
   end
   local f = sc:faction()
   if f then
      local ft = f:tags()
      if ft.generic or ft.misn_cargo then
         return true
      end
   end
   return notcomputer or misn_test.computer()
end

--[[--
   @brief Test for normal mission computer missions.
--]]
function misn_test.computer()
   local st = spob.cur():tags()
   local chance = 1
   if st.poor then
      chance = chance * 0.4
   end
   if st.refugee then
      chance = chance * 0.4
   end
   if chance < 1 and rnd.rnd() > chance then
      return false
   end
   return true
end

--[[--
   @brief Test for mercenary missions.
--]]
function misn_test.mercenary( notcomputer )
   if player.outfitNum("Mercenary Licence") <= 0 then
      return false
   end
   return notcomputer or misn_test.computer()
end

--[[--
   @brief Test for mercenary missions.
--]]
function misn_test.heavy_weapons( notcomputer )
   if player.outfitNum("Heavy Weapon Licence") <= 0 then
      return false
   end
   return notcomputer or misn_test.computer()
end

--[[--
   @brief Test for mercenary missions.
--]]
function misn_test.heavy_combat_vessel( notcomputer )
   if player.outfitNum("Heavy Combat Vessel Licence") <= 0 then
      return false
   end
   return notcomputer or misn_test.computer()
end

--[[--
   @brief Reweights mission chance based on number of unique active missions
--]]
function misn_test.reweight_active()
   local n = 0
   for k,v in ipairs(player.missions()) do
      if v.unique then
         n = n+1
      end
   end
   local HIGH = 8 -- Value at which mission is no longer given
   local LOW  = 3 -- Value at which mission is always given
   return (1-(n-LOW)/(HIGH-LOW) > rnd.rnd())
end

return misn_test
