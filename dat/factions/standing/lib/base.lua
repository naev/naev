--[[
   Simple skeleton for your standard faction. This is more or less what the
   standard behaviour can be, but from here you can let your imagination go
   wild.
--]]
local fmt = require "format"
local sbase = {}
friendly_at = 70 -- global

local function rep_from_points( points )
   -- 4 points for 20 point Llama, 30 points for 150 point Hawking
   return points / 5
end

function sbase.init( args )
   args = args or {}
   sbase.fct                = args.fct                              -- The faction

   local function param( name, def )
      sbase[name] = args[name] or def
   end

   -- Some general faction parameters
   param( "hit_range",     2 ) -- Range at which it affects
   param( "rep_min",       -100 )
   param( "rep_max",       30 )
   param( "secondary_default", 0.5 )
   param( "rep_max_var",   nil ) -- Mission variable to use for limits if defined
   param( "rep_from_points", rep_from_points )

   -- Type of source parameters.
   param( "destroy_max",   30 )
   param( "destroy_min",   -100 )
   param( "destroy_mod",   1 )

   --param( "disable_max",   20 )
   --param( "disable_min",   -100 )
   --param( "disable_mod",   0.3 )

   param( "board_max",     20 )
   param( "board_min",     -100 )
   param( "board_mod",     0.25 )

   param( "capture_max",   30 )
   param( "capture_min",   -100 )
   param( "capture_mod",   1 )

   param( "distress_max",  -20 ) -- Can't get positive  reputation from distress
   param( "distress_min",  -40 )
   param( "distress_mod",  0 )

   param( "scan_max",      -100 ) -- Can't gain reputation scanning by default
   param( "scan_min",      -20 )
   param( "scan_mod",      0.1 )

   -- Allows customizing relationships with other factions
   param( "attitude_toward", {} )

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

   -- TODO maybe we should do something better than this
   -- Look at enemy factions and remove them if they don't share systems
   if sbase.fct then
      for i,e in ipairs(sbase.fct:enemies()) do
         if sbase.attitude_toward[ e:nameRaw() ]==nil then
            local found = false
            for j,s in ipairs(system:getAll()) do
               if s:presence(e) > 0 and s:presence(sbase.fct) > 0 then
                  found = true
                  break
               end
            end
            if not found then
               sbase.attitude_toward[ e:nameRaw() ] = 0
            end
         end
      end
   end

   return sbase
end

-- based on GLSL clamp
local function clamp( x, min, max )
   return math.max( min, math.min( max, x ) )
end

-- Applies a local hit to a system
local function hit_local( sys, mod, min, max )
   -- Case system and no presence, it doesn't actually do anything...
   if sys and sys:presence( sbase.fct )<=0 then
      return
   end
   -- Just simple application based on local reputation
   local r = sys:reputation( sbase.fct )
   max = math.max( r, max ) -- Don't lower under the current value
   min = math.min( r, min ) -- Don't increase the current value
   local f = clamp( r+mod, min, max )
   sys:setReputation( sbase.fct, f )
   return f-r
end

-- Determine max and modifier based on type and whether is secondary
local function hit_mod( mod, source, secondary, primary_fct )
   local max, min

   -- Split by type
   if source=="destroy" then
      max = sbase.destroy_max
      min = sbase.destroy_min
      mod = sbase.destroy_mod * sbase.rep_from_points( mod )
   elseif source=="board" then
      max = sbase.board_max
      min = sbase.board_min
      mod = sbase.board_mod * sbase.rep_from_points( mod )
   elseif source=="capture" then
      max = sbase.capture_max
      min = sbase.capture_min
      mod = sbase.capture_mod * sbase.rep_from_points( mod )
   elseif source=="distress" then
      max = sbase.distress_max
      min = sbase.distress_min
      mod = sbase.distress_mod * sbase.rep_from_points( mod )
   elseif source=="scan" then
      max = sbase.scan_max
      min = sbase.scan_min
      mod = sbase.scan_mod * sbase.rep_from_points( mod )
   elseif source=="script" then -- "script" type is handled here
      max = reputation_max()
      min = sbase.rep_min
      --mod = mod -- Not modified
   else
      max = reputation_max()
      min = sbase.rep_min
      warn(fmt.f("Unknown faction hit source '{src}' for faction '{fct}'!"), {src=source,fct=sbase.fct})
   end

   -- Make sure it's within the global minimum and maximum.
   max = math.min( max, reputation_max() )
   min = math.max( min, sbase.rep_min )

   -- Modify secondaries
   if secondary ~= 0 then
      -- If we have a particular attitude towards a government, expose that
      local at = sbase.attitude_toward[ primary_fct:nameRaw() ]
      if at then
         mod = mod * at
      else
         mod = mod * sbase.secondary_default
      end
   end

   return min, max, mod
end

--[[
Handles a faction hit for a faction.

Possible sources:
   - "destroy": Pilot death.
   - "disable": Pilot ship was disabled.
   - "board": Pilot ship was boarded.
   - "capture": Pilot ship was captured.
   - "distress": Pilot distress signal.
   - "scan": when scanned by pilots and illegal content is found
   - "script": Either a mission or an event.

   @param sys System (or nil for global) that is having the hit
   @param mod Amount of faction being changed.
   @param source Source of the faction hit.
   @param secondary Flag that indicates whether this is a secondary hit. If 0 it is primary, if +1 it is secondary hit from ally, if -1 it is a secondary hit from an enemy.
   @param primary_fct In the case of a secondary hit, the faction that caused the primary hit.
   @return The faction amount to set to.
--]]
function hit( sys, mod, source, secondary, primary_fct )
   local min, max
   min, max, mod = hit_mod( mod, source, secondary, primary_fct )

   -- Case nothing changes, or too small to matter
   if math.abs(mod) < 1e-1 then
      return 0
   end

   -- No system, so just do the global hit
   if not sys then
      local changed
      if mod < 0 then
         changed = math.huge
      else
         changed = -math.huge
      end
      -- Apply change to all systems
      local minsys, maxsys
      local minval, maxval = math.huge, -math.huge
      for k,s in ipairs(system.getAll()) do
         local r = s:reputation( sbase.fct )
         if r < minval then
            minsys = s
            minval = r
         end
         if r > maxval then
            maxsys = s
            maxval = r
         end
         local fmin = math.min( r, min )
         local fmax = math.max( r, max )
         local f = clamp( r+mod, fmin, fmax )
         if mod < 0 then
            changed = math.min( changed, f-r )
         else
            changed = math.max( changed, f-r )
         end
         s:setReputation( sbase.fct, f )
      end

      -- Now propagate the thresholding from the max or min depending on sign of mod
      if mod >= 0 then
         sys = maxsys
      else
         sys = minsys
      end
      sbase.fct:applyLocalThreshold( sys )
      return changed
   end

   -- Center hit on sys and have to expand out
   local val = hit_local( sys, mod, min, max )
   local valsys = sys
   if sbase.hit_range > 0 then
      local done = { sys }
      local todo = { sys }
      for dist=1,sbase.hit_range do
         local dosys = {}
         for i,s in ipairs(todo) do
            for j,n in ipairs(s:adjacentSystems()) do
               if not inlist( done, n ) then
                  local v = hit_local( n, mod / (dist+1), min, max )
                  if not val then
                     val = v
                     valsys = n
                  end
                  table.insert( done, n )
                  table.insert( dosys, n )
               end
            end
         end
         todo = dosys
      end
   end

   -- Update frcom system that did hit and return change at that system
   if val then
      sbase.fct:applyLocalThreshold( valsys )
   end
   return val or 0
end

--[[
Highly simplified version that doesn't take into account maximum standings and the likes.
--]]
function hit_test( _sys, mod, source )
   local  _max
   _max, mod = hit_mod( mod, source, 0 )
   return mod
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
   if sbase.rep_max_var == nil then
      return sbase.rep_max
   end
   local max   = var.peek( sbase.rep_max_var )
   if max == nil then
      max = sbase.rep_max
      var.push( sbase.rep_max_var, max )
   end
   return max
end

return sbase
