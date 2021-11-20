--[[
   Attack wrappers for calling the correct attack functions.

   Here we set up the wrappers and determine exactly how the pilot should behave.

   The global layout is:
    - atk_util : Attack generic utilities.
    - atk_target : Targeting utilities.
--]]

-- Attack profiles
local atk_generic = require "ai.core.attack.generic"
local atk_fighter = require "ai.core.attack.fighter"
local atk_bomber = require "ai.core.attack.bomber"
local atk_corvette = require "ai.core.attack.corvette"
local atk_capital = require "ai.core.attack.capital"
--local atk_cruiser = require "ai.core.attack.cruiser"
--local atk_carrier = require "ai.core.attack.carrier"
local atk_drone = require "ai.core.attack.drone"

-- Set attack variables
mem.atk_changetarget  = 2 -- Distance at which target changes
mem.atk_approach      = 1.4 -- Distance that marks approach
mem.atk_aim           = 1.0 -- Distance that marks aim
mem.atk_board         = false -- Whether or not to board the target
mem.atk_kill          = true -- Whether or not to finish off the target
mem.atk_minammo       = 0.1 -- Percent of ammo necessary to do ranged attacks
mem.ranged_ammo       = 0 -- How much ammo is left, we initialize to 0 here just in case
mem.aggressive        = true --whether to take the more aggressive or more evasive option when given
mem.recharge          = false --whether to hold off shooting to avoid running dry of energy

--[[
-- Wrapper for the think functions.
--]]
function attack_think( target, si )
   -- Ignore other enemies
   if si.noattack then return end

   -- Update some high level stats
   mem.ranged_ammo = ai.getweapammo(4)

   if mem.atk_think ~= nil then
      mem.atk_think( target, si )
   else
      atk_generic.think( target, si )
   end
end


--[[
-- Wrapper for the attack functions.
--]]
-- luacheck: globals attack (AI Task functions passed by name)
function attack( target )
   -- Don't go on the offensive when in the middle of cooling.
   if mem.cooldown then
      ai.poptask()
      return
   end

   if mem.atk ~= nil then
      mem.atk( target )
   else
      atk_generic.atk( target )
   end
end

--[[
-- Forced attack function that should focus on the enemy until done
--]]
function attack_forced( target )
   if mem.atk ~= nil then
      mem.atk( target )
   else
      atk_generic.atk( target )
   end
end

--[[
-- Forced attack function that should focus on the enemy until enemy is killed
--]]
-- luacheck: globals attack_forced_kill (AI Task functions passed by name)
function attack_forced_kill( target )
   if mem.atk ~= nil then
      mem.atk( target, true )
   else
      atk_generic.atk( target, true )
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

   -- Set initial variables
   mem.ranged_ammo = ai.getweapammo(4)

   -- Lighter ships
   if class == "Bomber" then
      atk_bomber.init()

   elseif class == "Interceptor" then
      atk_drone.init()

   elseif class == "Fighter" then
      atk_fighter.init()

   -- Medium ships
   elseif class == "Corvette" then
      atk_corvette.init()

   -- Capital ships
   elseif class == "Destroyer" then
      atk_capital.init()

   elseif class == "Cruiser" then
      atk_capital.init()

   elseif class == "Battleship" then
      atk_capital.init()

   elseif class == "Carrier" then
      atk_capital.init()

    -- Generic AI
   else
      atk_generic.init()
   end
end

