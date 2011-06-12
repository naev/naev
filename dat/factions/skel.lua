--[[
   Simple skeleton for your standard faction. This is more or less what the
   standard behaviour can be, but from here you can let your imagination go
   wild.
--]]

-- Faction caps
_fcap_distress = 10 -- Distress cap
_fcap_kill     = 20 -- Kill cap
_fcap_misn     = 30 -- Starting mission cap, gets overwritten


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
   @brief Handles a faction hit for a faction.

   Possible sources:
    - "kill" : Pilot death.
    - "distress" : Pilot distress signal.
    - "script" : Either a mission or an event.

      @param current Current faction player has.
      @param amount Amount of faction being changed.
      @param source Source of the faction hit.
      @return The faction amount to set to.
--]]
function faction_hit( current, amount, source, secondary )
   -- Comfort macro
   local f = current

   -- Set cap based on source
   local cap
   local mod = 1
   if source == "distress" then
      cap   = _fcap_distress
      -- Ignore positive distresses
      if amount > 0 then
         return f
      end
   elseif source == "kill" then
      cap   = _fcap_kill
   else
      cap   = _fcap_misn
      --[[
      cap   = var.peek( "faction_misn" )
      if cap == nil then
         cap = _fcap_misn
         var.push( "faction_misn", cap )
      end
      --]]
   end

   -- Modify amount
   if secondary then mod = mod * 0.5 end
   amount = mod * amount

   -- Faction gain
   if amount > 0 then
      -- Must be under cap
      if f < cap then
         f = math.min( cap, f + amount * clerp( f, 0, 1, cap, 0.2 ) )
      end
   -- Faction loss
   else
      f = math.max( -100, f + amount * clerp( f, cap, 1, -100, 0.2 ) )
   end
   return f
end



