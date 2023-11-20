--[[
-- Common Pirate Mission framework
--
-- This framework allows to keep consistency and abstracts around commonly used
--  Pirate mission functions.
--]]
local fmt = require 'format'
local pir = {}

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

local fpir = faction.get("Pirate")
local fmar = faction.get("Marauder")

local _prefix = {
   ["Raven Clan"]    = _("RAVEN CLAN: "),
   ["Wild Ones"]     = _("WILD ONES: "),
   ["Dreamer Clan"]  = _("DREAMER CLAN: "),
   ["Black Lotus"]   = _("BLACK LOTUS: "),
}
function pir.prefix( fct )
   local p = _prefix[ fct:nameRaw() ]
   if not p then
      p = _("PIRATE: ")
   end
   return "#H"..p.."#0"
end

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

--[[
   @brief Gets whether or not a faction is a pirate faction
--]]
function pir.factionIsPirate( f )
   if not f then return false end
   return inlist( pir.factions, faction.get(f) )
end

--[[
   @brief Gets whether or not a faction is a pirate clan
--]]
function pir.factionIsClan( f )
   if not f then return false end
   return inlist( pir.factions_clans, faction.get(f) )
end

--[[
   @brief Computes the total amount of pirate-related factions in a system.
--]]
function pir.systemPresence( sys )
   sys = sys or system.cur()
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
   local pnt = spob.cur()
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
   return f or fpir
end

--[[
   @brief Probabilistically determines the dominant clan (treats the presence values as likelihoods).
--]]
function pir.systemClanP( sys )
   sys = sys or system.cur()
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
   return fpir
end

--[[
   @brief Gets a simple reputation message telling the player how the mission will increase their standing.
--]]
function pir.reputationMessage( f )
   if not pir.factionIsClan( f ) then
      return ""
   end
   return fmt.f(_(" This mission will increase your reputation with {fct_longname}."), {fct_longname=f:longname()})
end

--[[
   @brief Decrease pirate standings for doing normal missions.
--]]
function pir.reputationNormalMission( amount )
   for k,v in ipairs(pir.factions_clans) do
      local s = v:playerStanding()
      local d = v:playerStandingDefault()
      -- TODO Probably should handle this minimum stuff better
      local vamount = -amount
      if s > d then
         if s > 30 then
            vamount = vamount * 3
         elseif s > 0 then
            vamount = vamount * 2
         end
         v:modPlayerSingle( vamount )
      end
   end
end

--[[
   @brief Gets whether or not the pilot is in a pirate ship
--]]
function pir.isPirateShip( p )
   return p:ship():tags().pirate
end

--[[
   @brief Gets the maximum standing the player has with any clan
--]]
function pir.maxClanStanding ()
   local maxval = -100
   for k,v in ipairs(pir.factions_clans) do
      local vs = v:playerStanding()
      maxval = math.max( maxval, vs )
   end
   return maxval
end

--[[
   @brief Updates the standing of the marauders and pirates based on maxval (computed as necessary)
--]]
function pir.updateStandings( maxval )
   local pp = player.pilot()
   if not pp then return end
   maxval = maxval or pir.maxClanStanding()
   if pir.isPirateShip( pp ) then
      fpir:setPlayerStanding( maxval )
      fmar:setPlayerStanding( maxval - 20 )
   else
      fpir:setPlayerStanding( maxval - 20 )
      fmar:setPlayerStanding( maxval - 40 )
   end
end

--[[
   @brief Clears pirate pilots and stops them from spawning.
   @param onlynatural Whether or not to only clear natural pilots.
--]]
function pir.clearPirates( onlynatural )
   if not onlynatural then
      pilot.clearSelect( pir.factions )
      pilot.toggleSpawn( pir.factions, false )
   else
      for k,p in ipairs(pilot.get{ pir.factions }, true) do
         local m = p:memory()
         if  m.natural then
            p:rm()
         end
      end
      pilot.toggleSpawn( pir.factions, false )
   end
end

return pir
