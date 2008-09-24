--[[
-- Attack wrappers for calling the correct attack functions.
--]]


include("ai/include/attack_generic.lua")
include("ai/include/attack_bomber.lua")


atk_think = true


--[[
-- Wrapper for the think functions.
--]]
function attack_think ()
   -- Not everyone wants to think.
   if atk_think == false then
      return
   end

   if mem.atk_think ~= nil then
      mem.atk_think()
   else
      atk_g_think()
   end
end


--[[
-- Wrapper for the attack functions.
--]]
function attack ()
   if mem.atk ~= nil then
      mem.atk()
   else
      atk_g()
   end
end


-- [[
-- Generic function to choose what attack functions match the ship best.
-- ]]
function attack_choose ()
   class = ai.shipclass()

   --[[
   if class == "Bomber" then
      mem.atk_think = atk_b_think
      mem.atk = atk_b
   end
   ]]
end

