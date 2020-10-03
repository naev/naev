--[[
   Attack wrappers for calling the correct attack functions.

   Here we set up the wrappers and determine exactly how the pilot should behave.

   The global layout is:
    - atk_util.lua : Attack generic utilities.
    - atk_target.lua : Targetting utilities.
--]]

-- Utilities
require("ai/include/atk_util.lua")
require("ai/include/atk_target.lua")

-- Attack profiles
require("ai/include/atk_generic.lua")
require("ai/include/atk_fighter.lua")
require("ai/include/atk_bomber.lua")
require("ai/include/atk_corvette.lua")
require("ai/include/atk_capital.lua")
--require("ai/include/atk_cruiser.lua")
--require("ai/include/atk_carrier.lua")
require("ai/include/atk_drone.lua")

-- Set attack variables
mem.atk_changetarget  = 2 -- Distance at which target changes
mem.atk_approach      = 1.4 -- Distance that marks approach
mem.atk_aim           = 1.0 -- Distance that marks aim
mem.atk_board         = false -- Whether or not to board the target
mem.atk_kill          = true -- Whether or not to finish off the target
mem.aggressive        = true --whether to take the more aggressive or more evasive option when given
mem.recharge          = false --whether to hold off shooting to avoid running dry of energy


--[[
-- Wrapper for the think functions.
--]]
function attack_think ()
   if mem.atk_think ~= nil then
      mem.atk_think()
   else
      atk_generic_think()
   end
end


--[[
-- Wrapper for the attack functions.
--]]
function attack ()
   -- Don't go on the offensive when in the middle of cooling.
   if mem.cooldown then
      ai.poptask()
      return
   end

   if mem.atk ~= nil then
      mem.atk()
   else
      atk_generic()
   end
end


--[[
-- Wrapper for the attacked function.
--]]
function attack_attacked( attacker )
   if mem.atk_attacked ~= nil then
      mem.atk_attacked( attacker )
   else
      atk_generic_attacked( attacker )
   end
end


-- [[
-- Generic function to choose what attack functions match the ship best.
-- ]]
function attack_choose ()
   local class = ai.pilot():ship():class()

   -- Lighter ships
   if class == "Bomber" then
      atk_bomber_init()

   elseif class == "Fighter" then
      atk_fighter_init()

   elseif class == "Drone" then
      atk_drone_init()

   -- Medium ships
   elseif class == "Corvette" then
      atk_corvette_init()

   -- Capital ships
   elseif class == "Destroyer" then
      atk_capital_init()

   elseif class == "Cruiser" then
      atk_capital_init()

   elseif class == "Carrier" then
      atk_capital_init()

    -- Generic AI
   else
      atk_generic_init()
   end
end

