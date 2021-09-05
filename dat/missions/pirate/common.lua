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
   local floor = var.peek("_ffloor_decay_pirate") or -20
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
-- List of all the pirate clan factions
pir.factions_clans = {
   faction.get("Raven Clan"),
   faction.get("Wild Ones"),
   faction.get("Dreamer Clan"),
   faction.get("Black Lotus"),
}

--[[
   @brief Gets whether or not a faction is a pirate faction
--]]
function pir.factionIsPirate( f )
   if not f then return false end
   f = faction.get(f)
   for k,v in ipairs(pir.factions) do
      if f==v then
         return true
      end
   end
   return false
end

--[[
   @brief Gets whether or not a faction is a pirate clan
--]]
function pir.factionIsClan( f )
   if not f then return false end
   f = faction.get(f)
   for k,v in ipairs(pir.factions_clans) do
      if f==v then
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
      total = total + (p[v:nameRaw()] or 0)
   end
   return total
end

--[[
   @brief Gets the dominant clan of a system.
--]]
function pir.systemClan( sys )
   sys = sys or system.cur()
   -- Return faction of landed asset if applicable
   local pnt = planet.cur()
   if pnt then
      local pfact = pnt:faction()
      if pfact and pir.factionIsPirate( pfact ) then
         return pfact
      end
   end

   local f
   local m = 0
   local p = sys:presences()
   for k,v in ipairs(pir.factions_clans) do
      local pp = p[v:nameRaw()]
      if pp and pp > m then
         f = v
         m = pp
      end
   end
   return f or faction.get("Pirate")
end

--[[
   @brief Probabilistically determines the dominant clan (treats the presence values as likelihoods).
--]]
function pir.systemClanP( sys )
   sys = sys or system.cur()
   -- Return faction of landed asset if applicable
   local pnt = planet.cur()
   if pnt then
      local pfact = pnt:faction()
      if pfact and pir.factionIsPirate( pfact ) then
         return pfact
      end
   end

   local total = 0
   local p = sys:presences()
   for k,v in ipairs(pir.factions_clans) do
      total = total + (p[v:nameRaw()] or 0)
   end
   local r = rnd.rnd()
   local accum = 0
   for k,v in ipairs(pir.factions_clans) do
      local pp = p[v:nameRaw()] or 0
      accum = accum + pp
      if r < accum / total then
         return v
      end
   end
   return faction.get("Pirate")
end

--[[
   @brief Gets a simple reputation message telling the player how the mission will increase their standing.
--]]
function pir.reputationMessage( f )
   return string.format(_("This mission will increase your reputation with %s."), f:longname())
end

--[[
   @brief Decrease pirate standings for doing normal missions.
--]]
function pir.reputationNormalMission( amount )
   for k,v in ipairs(pir.faction_clans) do
      local s = v:playerStanding()
      -- TODO Probably should handle this minimum stuff better
      local vamount = -amount
      if s > -50 then
         if s > 30 then
            vamount = vamount * 3
         elseif s > 0 then
            vamount = vamount * 2
         end
         v:modPlayerSingle( vamount )
      end
   end
end

return pir
