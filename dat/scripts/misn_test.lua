--[[

   Simple library to handle common conditional expressions, mainly focused on
   mission computer missions.

--]]
local misn_test = {}

--[[--
   @brief Test for cargo missions.
--]]
function misn_test.cargo()
   if not misn_test.computer() then
      return false
   end

   -- Has to support normal factions
   local f = spob.cur():faction()
   if f then
      local ft = f:tags()
      if ft.generic or ft.misn_cargo then
         return true
      end
   end
   return false
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
function misn_test.mercenary()
   if player.outfitNum("Mercenary License") <= 0 then
      return false
   end
   return misn_test.computer()
end

--[[--
   @brief Reweights mission chance based on number of uinque active missions
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
