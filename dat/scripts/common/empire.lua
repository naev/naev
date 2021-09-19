--[[
-- Common Empire Mission framework
--
-- This framework allows to keep consistency and abstracts around commonly used
--  empire mission functions.
--]]
local emp = {}

--[[
   @brief Increases the reputation limit of the player.
--]]
function emp.modReputation( increment )
   local cur = var.peek("_fcap_empire") or 30
   var.push( "_fcap_empire", math.min(cur+increment, 100) )
end

function emp.addShippingLog( text )
   shiplog.create("empire_shipping", _("Empire Shipping"), _("Empire"))
   shiplog.append("empire_shipping", text)
end

function emp.addCollectiveLog( text )
   shiplog.create("empire_collective", _("Empire Collective Campaign"), _("Empire"))
   shiplog.append("empire_collective", text)
end

return emp
