--[[
-- Common Pirate Mission framework
--
-- This framework allows to keep consistency and abstracts around commonly used
--  Pirate mission functions.
--]]


--[[
   @brief Increases the reputation limit of the player.
--]]
function pir_modReputation( increment )
   local cur = var.peek("_fcap_pirate") or 30
   var.push( "_fcap_pirate", math.min(cur+increment, 100) )
end


--[[
   @brief Increases the decay floor (how low reputation can decay to).
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
