--[[
   Simple skeleton for your standard faction. This is more or less what the
   standard behaviour can be, but from here you can let your imagination go
   wild.
--]]
local class = require "class"

local sbase = {}

sbase.Standing = class.inheritsFrom()
function sbase.newStanding( args )
   return sbase.Standing.new():init( args )
end

function sbase.Standing:init( args )
   self.fct               = args.fct                              -- The faction

   -- Faction caps.
   self.cap_kill           = args.cap_kill            or 20       -- Kill cap
   self.delta_distress     = args.delta_distress      or {-1, 0}  -- Maximum change constraints
   self.delta_kill         = args.delta_kill          or {-5, 1}  -- Maximum change constraints
   self.cap_misn_def       = args.cap_misn_def        or 30       -- Starting mission cap, gets overwritten
   self.cap_misn_var       = args.cap_misn_var        or nil      -- Mission variable to use for limits

   -- Secondary hit modifiers.
   self.mod_distress_enemy = args.mod_distress_enemy  or 0        -- Distress of the faction's enemies
   self.mod_distress_friend= args.mod_distress_friend or 0.3      -- Distress of the faction's allies
   self.mod_kill_enemy     = args.mod_kill_enemy      or 0.3      -- Kills of the faction's enemies
   self.mod_kill_friend    = args.mod_kill_friend     or 0.3      -- Kills of the faction's allies
   self.mod_misn_enemy     = args.mod_misn_enemy      or 0.3      -- Missions done for the faction's enemies
   self.mod_misn_friend    = args.mod_misn_friend     or 0.3      -- Missions done for the faction's allies

   self.friendly_at        = args.friendly_at          or 70       -- Standing value threshold between neutral and friendly.

   -- Text stuff
   self.text = args.text or {
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
   self.text_friendly = args.text_friendly or _("Friendly")
   self.text_neutral  = args.text_neutral or _("Neutral")
   self.text_hostile  = args.text_hostile or _("Hostile")
   self.text_bribed   = args.text_bribed or _("Bribed")
   return self
end


--[[
   Clamps a value x between low and high.
--]]
local function clamp( low, high, x )
   return math.max( low, math.min( high, x ) )
end


--[[
   Linearly interpolates x between x1,y1 and x2,y2
--]]
local function lerp( x, x1, y1, x2, y2 )
   local m = (y1-y2) / (x1-x2)
   local b = y1-m*x1
   return m*x + b
end


--[[
   Same as lerp but clamps to [0,1].
--]]
local function clerp( x, x1, y1, x2, y2 )
   return clamp( 0, 1, lerp( x, x1, y1, x2, y2 ) )
end


--[[
   Duplicates a table to avoid clobbering.
--]]
local function clone(t)
   local new = {}
   for k, v in pairs(t) do
      new[k] = v
   end
   return new
end


--[[
   Handles a faction hit for a faction.

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
function sbase.Standing:hit( current, amount, source, secondary )
   -- Comfort macro
   local f = current
   local delta = {-200, 200}

   -- Set caps and/or deltas based on source
   local cap
   local mod = 1
   if source == "distress" then
      cap   = self.cap_kill
      delta = clone( self.delta_distress )

      -- Adjust for secondary hit
      if secondary then
         if amount > 0 then
            mod = mod * self.mod_distress_enemy
         else
            mod = mod * self.mod_distress_friend
         end
      end
   elseif source == "kill" then
      cap   = self.cap_kill
      delta = clone(self.delta_kill)

      -- Adjust for secondary hit
      if secondary then
         if amount > 0 then
            mod = mod * self.mod_kill_enemy
         else
            mod = mod * self.mod_kill_friend
         end
      end
   else
      cap = self:reputation_max()

      -- Adjust for secondary hit
      if secondary then
         if amount > 0 then
            mod = mod * self.mod_misn_friend
         else
            mod = mod * self.mod_misn_enemy
         end
      end
   end

   amount = mod * amount
   delta[1] = mod * delta[1]
   delta[2] = mod * delta[2]

   -- Faction gain
   if amount > 0 then
      -- Must be under cap
      if f < cap then
         if source == "kill" then
            local has_planet
            -- Positive kill, which means an enemy of this faction got killed.
            -- We need to check if this happened in the faction's territory, otherwise it doesn't count.
            for _k, planet in ipairs(system.cur():spobs()) do
                if planet:faction() == self.fct then
                   -- Planet belonging to this faction found. Modify reputation.
                   f = math.min( cap, f + math.min(delta[2], amount * clerp( f, 0, 1, cap, 0.2 )) )
                   has_planet = true
                   break
                end
            end
            local witness = pilot.get( { self.fct } )
            if not has_planet and witness then
               for _k, pilot in ipairs(witness) do
                  if player.pos():dist( pilot:pos() ) < 5000 then
                     -- Halve impact relative to a normal secondary hit.
                     f = math.min( cap, f + math.min(delta[2], amount * 0.5 * clerp( f, 0, 1, cap, 0.2 )) )
                     break
                  end
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


--[[
   Returns a text representation of the player's standing.

      @param value Current standing value of the player.
      @return The text representation of the current standing.
--]]
function sbase.Standing:text_rank( value )
   for i = math.floor( value ), 0, ( value < 0 and 1 or -1 ) do
      if self.text[i] ~= nil then
         return self.text[i]
      end
   end
   return self.text[0]
end


--[[
   Returns a text representation of the player's broad standing.

      @param value Current standing value of the player.
      @param bribed Whether or not the respective pilot is bribed.
      @param override If positive it should be set to ally, if negative it should be set to hostile.
      @return The text representation of the current broad standing.
--]]
function sbase.Standing:text_broad( value, bribed, override )
   if override == nil then override = 0 end

   if bribed then
      return self.text_bribed
   elseif override > 0 or value >= standing.friendly_at then
      return self.text_friendly
   elseif override < 0 or value < 0 then
      return self.text_hostile
   else
      return self.text_neutral
   end
end

--[[
   Returns the maximum reputation limit of the player.
--]]
function sbase.Standing:reputation_max()
   if self.cap_misn_var == nil then
      return self.cap_misn_def
   end

   local cap   = var.peek( self.cap_misn_var )
   if cap == nil then
      cap = self.cap_misn_def
      var.push( self.cap_misn_var, cap )
   end
   return cap
end

return sbase
