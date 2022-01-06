local dv = {}

dv.prefix = "#o".._("DV: ").."#0" -- Repeatable mission prefix

--[[
   @brief Increases the reputation limit of the player.
--]]
function dv.modReputation( increment )
   local cur = var.peek("_fcap_dvaered") or 30
   var.push("_fcap_dvaered", math.min(cur+increment, 100) )
end

function dv.addAntiFLFLog( text )
   shiplog.create( "dv_antiflf", _("Anti-FLF Campaign"), _("Dvaered") )
   shiplog.append( "dv_antiflf", text )
end

function dv.addStandardLog( text )
   shiplog.create("dvaered_standard", _("Dvaered Standard"), _("Dvaered"))
   shiplog.append("dvaered_standard", text)
end

return dv
