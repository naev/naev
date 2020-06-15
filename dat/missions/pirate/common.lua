--[[
-- Common Pirate Mission framework
--
-- This framework allows to keep consistency and abstracts around commonly used
--  Pirate mission functions.
--]]


function pir_modDecayFloor( n )
   local floor = var.peek("_ffloor_decay_pirate")
   if floor == nil then floor = -20 end
   floor = math.min(floor + n, -1)
   var.push("_ffloor_decay_pirate", floor)
end
