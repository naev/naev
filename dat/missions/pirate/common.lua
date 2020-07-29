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


function pir_addMiscLog( text )
   shiplog.createLog("pir_misc", _("Miscellaneous"), _("Pirate"))
   shiplog.appendLog("pir_misc", text)
end
