--[[
-- Attack wrappers for calling the correct attack functions.
--]]


include("ai/include/attack_generic.lua")
include("ai/include/attack_bomber.lua")


--[[
-- Wrapper for the think functions.
--]]
function attack_think ()
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


--[[
-- Wrapper for the attacked function.
--]]
function attack_attacked( attacker )
   if mem.atk_attacked ~= nil then
      mem.atk_attacked( attacker )
   else
      atk_g_attacked( attacker )
   end
end


-- [[
-- Generic function to choose what attack functions match the ship best.
-- ]]
function attack_choose ()
   class = ai.shipclass()

   --[[]]
   if class == "Bomber" then
--      ai.comm(1, "Bomber")
      mem.atk_think = atk_topdown_think
      mem.atk = atk_bomber
      --mem.atk_think = atk_b_think
      --mem.atk = atk_b
    elseif class == "Fighter" or class == "Drone" then
      mem.atk_think = atk_fighter_think
      mem.atk = atk_fighter
    elseif class == "Corvette" then
--      ai.comm(1, "Corvette")
      mem.atk_think = atk_topdown_think
      mem.atk = atk_corvette
    elseif class == "Destroyer" or class == "Cruiser" then
--      ai.comm(1, "Capship")
      mem.atk_think = atk_topdown_think
      mem.atk = atk_capital
    else 
--      ai.comm(1, "Other")
      mem.atk_think = atk_g_think
      mem.atk = atk_g
   end
   
end

