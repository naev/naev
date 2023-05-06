local atk_generic = require "ai.core.attack.generic"
local flow = require "ships.lua.lib.flow"

local atk = {}

-- [[
-- Generic function to choose what attack functions match the ship best.
-- ]]
function atk.choose ()
   local p = ai.pilot()
   local class = p:ship():class()

   -- Set initial variables
   mem.ranged_ammo = ai.getweapammo(4)
   mem.equipopt_params = mem.equipopt_params or {}

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

   -- Initialize if necessary
   if mem.atk.init then
      mem.atk.init()
   end
end

--[[
-- Wrapper for the think functions.
--]]
function atk.think( target, si, noretarget )
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
         if rnd.rnd() < 0.3 and ai.dist( target ) < 3000 then
            p:outfitToggle( mem._o.hologram_projector, true )
         end
      end

      -- The bite
      if mem._o.bite and p:outfitReady( mem._o.bite )then
         if ai.dir( target ) < math.rad(20) then
            local dtime = 3
            if mem._o.bite_lust then
               dtime = dtime+2
            end
            local s = p:stats()
            if ai.dist( target ) < (s.speed + (s.thrust+800)/3)*dtime then
               p:outfitToggle( mem._o.bite, true )
            end
         end
      end

      -- Plasma Burst
      if mem._o.plasma_burst and p:outfitReady( mem._o.plasma_burst ) then
         if ai.dist( target ) < 300 then
            p:outfitToggle( mem._o.plasma_burst, true )
         else
            local e = p:getEnemies( 300 )
            if #e >= 2 then
               p:outfitToggle( mem._o.plasma_burst, true )
            end
         end
      end

      -- Check to see if we want to do flow stuff
      if mem._o.flow then
         local f = flow.get( p )
         local fm = flow.max( p )
         local s = flow.size( p )

         if mem._o.seeking_chakra and f > fm * 0.25 and rnd.rnd() < 0.3 then
            if ai.dist( target ) < 3000 then -- TODO more exact range
               p:outfitToggle( mem._o.seeking_chakra, true )
            end
         end
         if mem._o.feather_drive and f > fm * 0.25 and rnd.rnd() < 0.3 then
            if ai.dir( target ) < math.rad(10) and ai.dist( target ) < 300  then
               p:outfitToggle( mem._o.feather_drive, true )
            end
         end
         if mem._o.astral_projection and f >= fm * 0.45 and rnd.rnd() < 0.1 then
            if ai.dist( target ) < 1500 + s * 500 then
               p:outfitToggle( mem._o.astral_projection, true )
            end
         end
         if mem._o.cleansing_flames and f > fm * 0.25 and rnd.rnd() < 0.5 then
            if ai.dist( target ) < 400 then
               p:outfitToggle( mem._o.cleansing_flames, true )
            end
         end
         if mem._o.house_mirrors and f > fm * 0.4 and rnd.rnd() < 0.5 then
            -- Want enemy to be in shooting range
            local range = ai.getweaprange(3, 0) -- forward turrets
            if ai.dist( target ) < 0.8*range then
               p:outfitToggle( mem._o.house_mirrors, true )
            end
         end
         if mem._o.reality_rip and f > fm * 0.4 and rnd.rnd() < 0.5 then
            -- Want enemy to be close, but not too close
            local range = ai.getweaprange(3, 0) -- forward turrets
            local d = ai.dist( target )
            if d > 300 and d < 0.8*range  then
               p:outfitToggle( mem._o.house_mirrors, true )
            end
         end
      end
   end

   -- The think function basically tries to figure out new targets
   if not noretarget then
      local lib = (mem.atk or atk_generic)
      local func = (lib.think or atk_generic.think)
      func( target, si )
   end
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
