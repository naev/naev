--[[
   Simple skeleton for your standard faction. This is more or less what the
   standard behaviour can be, but from here you can let your imagination go
   wild.
--]]
local sbase = {}
friendly_at = 70 -- global

function sbase.init( args )
   args = args or {}
   sbase.fct                = args.fct                              -- The faction

   -- Faction caps.
   sbase.cap_kill           = args.cap_kill            or 20       -- Kill cap
   sbase.cap_misn_def       = args.cap_misn_def        or 30       -- Starting mission cap, gets overwritten
   sbase.cap_misn_var       = args.cap_misn_var        or nil      -- Mission variable to use for limits

   -- Secondary hit modifiers.
   sbase.mod_distress_enemy = args.mod_distress_enemy  or 0        -- Distress of the faction's enemies
   sbase.mod_distress_friend= args.mod_distress_friend or 0.3      -- Distress of the faction's allies
   sbase.mod_kill_enemy     = args.mod_kill_enemy      or 0.3      -- Kills of the faction's enemies
   sbase.mod_kill_friend    = args.mod_kill_friend     or 0.3      -- Kills of the faction's allies
   sbase.mod_misn_enemy     = args.mod_misn_enemy      or 0.3      -- Missions done for the faction's enemies
   sbase.mod_misn_friend    = args.mod_misn_friend     or 0.3      -- Missions done for the faction's allies

   -- Text stuff
   sbase.text = args.text or {
      [100] = _("Legend"),
      [90]  = _("Hero"),
      [70]  = _("Comrade"),
      [50]  = _("Ally"),
      [30]  = _("Partner"),
      [10]  = _("Associate"),
      [0]   = _("Neutral"),
      [-1]  = _("Outlaw"),
      [-30] = _("Criminal"),
      [-50] = _("Enemy"),
   }
   sbase.text_friendly = args.text_friendly or _("Friendly")
   sbase.text_neutral  = args.text_neutral or _("Neutral")
   sbase.text_hostile  = args.text_hostile or _("Hostile")
   sbase.text_bribed   = args.text_bribed or _("Bribed")
   return sbase
end

local function fctmod( sys, mod, cap, secondary, modenemy, modfriend )
   -- Make sure it doesn't go over the true cap
   cap = math.min( reputation_max(), cap )

   -- Adjust for secondary hit if applicable
   if secondary then
      if mod > 0 then
         mod = mod * modenemy
      else
         mod = mod * modfriend
      end
   end

   -- Being applied to a system
   if sys then
      local f = sys:reputation( sbase.fct ) + mod
      f = math.min( cap, f )
      sys:setReputation( sbase.fct, f )
      sbase.fct:applyLocalThreshold( sys )
   else
      -- Apply change to all systems
      local minsys, maxsys
      local minval, maxval = math.huge, -math.huge
      for k,s in ipairs(system.getAll()) do
         local f = s:reputation( sbase.fct )
         if f < minval then
            minsys = s
            minval = f
         end
         if f > maxval then
            maxsys = s
            maxval = f
         end
         s:setReputation( sbase.fct, f+mod )
      end

      -- Now propagate the thresholding from the max or min depending on sign of mod
      if mod >= 0 then
         sys = maxsys
      else
         sys = minsys
      end
      sbase.fct:applyLocalThreshold( sys )
   end

end

--[[
Handles a faction hit for a faction.

Possible sources:
   - "destroy": Pilot death.
   - "board": Pilot ship as boarded.
   - "capture": Pilot ship was captured.
   - "distress": Pilot distress signal.
   - "scan": when scanned by pilots and illegal content is found
   - "script": Either a mission or an event.

   @param sys System (or nil for global) that is having the hit
   @param mod Amount of faction being changed.
   @param source Source of the faction hit.
   @param secondary Flag that indicates whether this is a secondary (through ally or enemy) hit.
   @return The faction amount to set to.
--]]
function hit( sys, mod, source, secondary )
   if source=="distress" or source=="scan" then
      fctmod( sys, mod, sbase.cap_kill, secondary, sbase.mod_distress_enemy, sbase.mode_distress_friend )

   elseif source == "kill" or source=="board" or source=="capture"then
      fctmod( sys, mod, sbase.cap_kill, secondary, sbase.mod_kill_enemy, sbase.mod_kill_friend )

   else
      fctmod( sys, mod, reputation_max(), secondary, sbase.mod_misn_enemy, sbase.mod_misn_friend )
   end
end


--[[
Returns a text representation of the player's standing.

   @param value Current standing value of the player.
   @return The text representation of the current standing.
--]]
function text_rank( value )
   for i = math.floor( value ), 0, ( value < 0 and 1 or -1 ) do
      if sbase.text[i] ~= nil then
         return sbase.text[i]
      end
   end
   return sbase.text[0]
end


--[[
Returns a text representation of the player's broad standing.

   @param value Current standing value of the player.
   @param bribed Whether or not the respective pilot is bribed.
   @param override If positive it should be set to ally, if negative it should be set to hostile.
   @return The text representation of the current broad standing.
--]]
function text_broad( value, bribed, override )
   if override == nil then override = 0 end

   if bribed then
      return sbase.text_bribed
   elseif override > 0 or value >= friendly_at then
      return sbase.text_friendly
   elseif override < 0 or value < 0 then
      return sbase.text_hostile
   else
      return sbase.text_neutral
   end
end

--[[
   Returns the maximum reputation limit of the player.
--]]
function reputation_max ()
   if sbase.cap_misn_var == nil then
      return sbase.cap_misn_def
   end

   local cap   = var.peek( sbase.cap_misn_var )
   if cap == nil then
      cap = sbase.cap_misn_def
      var.push( sbase.cap_misn_var, cap )
   end
   return cap
end

return sbase
