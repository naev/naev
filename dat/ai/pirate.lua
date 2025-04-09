require 'ai.core.core'
require 'ai.core.idle.pirate'
local pirlib = require 'ai.core.misc.pirate'

--[[
   The former-Glorious Pirate AI, now externalized basically in a
   library
--]]

-- Settings
mem.aggressive    = true
mem.safe_distance = 500
mem.armour_run    = 80
mem.armour_return = 100
mem.atk_board     = true
mem.atk_kill      = false
mem.careful       = true

function create ()
   create_pre()
   pirlib.create()
   -- Lower skill than the clans
   mem.atk_skill  = 0.45 + 0.3*rnd.sigma()
   create_post()
end

hail = pirlib.hail

function taunt( target, offense )
   -- Only 50% of actually taunting.
   if rnd.rnd(0,1) == 0 then
      return
   end

   return pirlib.taunt( target, offense )
end
