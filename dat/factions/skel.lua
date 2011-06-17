--[[
   Simple skeleton for your standard faction. This is more or less what the
   standard behaviour can be, but from here you can let your imagination go
   wild.
--]]

-- Faction caps.
_fcap_kill       = 20 -- Kill cap
_fdelta_distress = {-1, 0} -- Maximum change constraints
_fdelta_kill     = {-5, 1} -- Maximum change constraints
_fcap_misn       = 30 -- Starting mission cap, gets overwritten
_fcap_misn_var   = nil -- Mission variable to use for limits
_fcap_mod_sec    = 0.3 -- Modulation from secondary


--[[
   @brief Clamps a value x between low and high.
--]]
function clamp( low, high, x )
   return math.max( low, math.min( high, x ) )
end


--[[
   @brief Linearly interpolates x between x1,y1 and x2,y2
--]]
function lerp( x, x1, y1, x2, y2 )
   local m = (y1-y2)/(x1-x2)
   local b = y1-m*x1
   return m*x + b
end


--[[
   @brief Same as lerp but clamps to [0,1].
--]]
function clerp( x, x1, y1, x2, y2 )
   return clamp( 0, 1, lerp( x, x1, y1, x2, y2 ) )
end


--[[
   @brief Duplicates a table to avoid clobbering.
--]]
function clone(t)
   local new = {}
   for k, v in pairs(t) do
      new[k] = v
   end
   return new
end


--[[
   @brief Handles a faction hit for a faction.

   Possible sources:
    - "kill" : Pilot death.
    - "distress" : Pilot distress signal.
    - "script" : Either a mission or an event.

      @param current Current faction player has.
      @param amount Amount of faction being changed.
      @param source Source of the faction hit.
      @param secondary Flag that indicates whether this is a secondary (through ally or enemy) hit.
      @return The faction amount to set to.
--]]
function default_hit( current, amount, source, secondary )
   -- Comfort macro
   local f = current
   local delta = {-200, 200}

   -- Set caps and/or deltas based on source
   local cap
   local mod = 1
   if source == "distress" then
      delta = clone(_fdelta_distress)
      -- Ignore positive distresses
      if amount > 0 then
         return f
      end
   elseif source == "kill" then
      cap   = _fcap_kill
      delta = clone(_fdelta_kill)
   else
      if _fcap_misn_var == nil then
         cap   = _fcap_misn
      else
         cap   = var.peek( _fcap_misn_var )
         if cap == nil then
            cap = _fcap_misn
            var.push( _fcap_misn_var, cap )
         end
      end
   end

   -- Adjust for secondary hit
   if secondary then mod = mod * _fcap_mod_sec end
   amount = mod * amount
   delta[1] = mod * delta[1]
   delta[2] = mod * delta[2]

   -- Faction gain
   if amount > 0 then
      -- Must be under cap
      if f < cap then
         if source == "kill" then
            -- Positive kill, which means an enemy of this faction got killed.
            -- We need to check if this happened in the faction's territory, otherwise it doesn't count.
            -- NOTE: virtual assets are NOT counted when determining territory!
            for _, planet in ipairs(system.cur():planets()) do
                if planet:faction() == _fthis then
                   -- Planet belonging to this faction found. Modify reputation.
                   f = math.min( cap, f + math.min(delta[2], amount * clerp( f, 0, 1, cap, 0.2 )) )
                   break
                end
            end
         else
            -- Script induced change. No diminishing returns on these.
            f = math.min( cap, f + math.min(delta[2], amount) )
         end
      end
   -- Faction loss.
   else
      -- No diminishing returns on loss.
      f = math.max( -100, f + math.max(delta[1], amount) )
   end
   return f
end
