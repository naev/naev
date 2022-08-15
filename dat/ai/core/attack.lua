local atk_generic = require "ai.core.attack.generic"

local atk = {}

-- [[
-- Generic function to choose what attack functions match the ship best.
-- ]]
function atk.choose ()
   local p = ai.pilot()
   local class = p:ship():class()

   -- Set initial variables
   mem.ranged_ammo = ai.getweapammo(4)

   -- Lighter ships
   if class == "Bomber" then
      mem.atk = require "ai.core.attack.bomber"

   elseif class == "Interceptor" then
      mem.atk = require "ai.core.attack.drone"

   elseif class == "Fighter" then
      mem.atk = require "ai.core.attack.fighter"

   -- Medium ships
   elseif class == "Corvette" then
      mem.atk = require "ai.core.attack.corvette"

   -- Capital ships
   elseif class == "Destroyer" or class == "Cruiser" or class == "Battleship" or class == "Carrier" then
      mem.atk = require "ai.core.attack.capital"

    -- Generic AI
   else
      mem.atk = atk_generic
   end
end

--[[
-- Wrapper for the think functions.
--]]
function atk.think( target, si )
   -- Ignore other enemies
   if si.noattack then return end

   -- Update some high level stats
   mem.ranged_ammo = ai.getweapammo(4)

   -- Use special outfits
   if mem._o then
      local p = ai.pilot()

      -- Use shield booster if applicable
      if mem._o.shield_booster then
         local _a, s = p:health()
         local e = p:energy()
         if s < 50 and e > 20 then
            p:outfitToggle( mem._o.shield_booster, true )
         end
      end

      -- Jam stuff
      if mem._o.jammer and ai.haslockon() then
         if p:energy() > 40 then
            p:outfitToggle( mem._o.jammer, true )
         end
      end

      -- Accelerate time
      if mem._o.neural_interface then
         if p:energy() > 20 then
            p:outfitToggle( mem._o.neural_interface, true )
         end
      end

      -- Combat holograms
      if mem._o.hologram_projector then
         p:outfitToggle( mem._o.hologram_projector )
      end
   end

   local lib = (mem.atk or atk_generic)
   local func = (lib.think or atk_generic.think)
   func( target, si )
end

--[[
-- Wrapper for the attacked function. Only called from "attack" tasks (i.e., under "if si.attack").
--]]
function atk.attacked( attacker )
   local lib = (mem.atk or atk_generic)
   local func = (lib.attacked or atk_generic.attacked)
   func( attacker )
end

return atk
