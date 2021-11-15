require 'ai.core.guard'

--[[
   Generic Mission baddie AI that tries to stay near a position
]]--

-- Settings
mem.aggressive     = true
mem.safe_distance  = 500
mem.armour_run     = 80
mem.armour_return  = 100
mem.guarddodist   = math.huge
mem.guardreturndist = math.huge

