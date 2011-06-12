



--[[
   @brief Increases the reputation limit of the player.
--]]
function dvp_modReputation( increment )
   local cur = var.peek("_fcap_dvaered") or 30
   var.push("_fcap_dvaered", math.min(cur+increment, 100) )
end



