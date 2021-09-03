--[[
-- Common Pirate Mission framework
--
-- This framework allows to keep consistency and abstracts around commonly used
--  Pirate mission functions.
--]]
local pir = {}

--[[
   @brief Increases the reputation limit of the player.
--]]
function pir.modReputation( increment )
   local cur = var.peek("_fcap_pirate") or 30
   var.push( "_fcap_pirate", math.min(cur+increment, 100) )
end

--[[
   @brief Increases the decay floor (how low reputation can decay to).
--]]
function pir.modDecayFloor( n )
   local floor = var.peek("_ffloor_decay_pirate")
   if floor == nil then floor = -20 end
   floor = math.min(floor + n, -1)
   var.push("_ffloor_decay_pirate", floor)
end

--[[
   @brief Adds miscellaneous pirate log entry.
--]]
function pir.addMiscLog( text )
   shiplog.create("pir_misc", _("Miscellaneous"), _("Pirate"))
   shiplog.append("pir_misc", text)
end

-- List of all the pirate factions
pir.factions = {
   faction.get("Pirate"),
   faction.get("Marauder"),
   faction.get("Raven Clan"),
   faction.get("Wild Ones"),
   faction.get("Dreamer Clan"),
   faction.get("Black Lotus"),
}

--[[
   @brief Gets whether or not a faction is a pirate faction
--]]
function pir.factionIsPirate( f )
   for k,v in ipairs(pir.factions) do
      if faction.get(f)==v then
         return true
      end
   end
   return false
end

--[[
   @brief Computes the total amount of pirate-related factions in a system.
--]]
function pir.systemPresence( sys )
   local total = 0
   local p = sys:presences()
   for k,v in ipairs(pir.factions) do
      total = total + (p[v] or 0)
   end
   return total
end

return pir
