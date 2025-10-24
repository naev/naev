--[[
   Library that supersedes "proximity" trying to rely less on globals and keep
   a more simple and less flexible API.

   Hooks in this library are not persistent across saves and meant for
   in-system events (usually triggered from a hook.enter() hook).

   @madule trigger
--]]
local lib = {}

function _hook_distance( params )
   if player.pos():dist2( params.target ) <= params.dist2 then
      params.fn()
   else
      hook.timer( 1, "_hook_distance", params )
   end
end

--[[--
Runs a function 'fn' when the player is within a distance of a target position.

   @tparam vec2 target Target position.
   @tparam number distance Distance from the target position to trigger.
   @tparam function fn Function to run when within the distance.
--]]
function lib.distance_player( target, distance, fn )
   local params = {
      target   = target,
      fn       = fn,
      dist2    = distance^2,
   }
   hook.timer( 1, "_hook_distance", params )
end

local function pilot_defeated( plt, params )
   -- Make disable permanent if necessary
   if params.permdisable and plt:disabled() then
      plt:setDisable()
   end

   -- Remove pilot from list of pilots
   local newplts = {}
   for k,p in ipairs(params.pilots) do
      if p~=plt and p:exists() then
         table.insert( newplts, p )
      end
   end
   params.pilots = newplts

   -- Run hook if necessary
   if #params.pilots <= 0 then
      params.fn()
   end
end
function _hook_pilot_defeated( plt, _killer, params )
   pilot_defeated( plt, params )
end
function _hook_pilot_exploded( plt, params )
   pilot_defeated( plt, params )
end

--[[--
Runs a function `fn` when all the pilots are defeated (either disabled or destroyed).

Disabled pilots will become permanently disabled so they can't "recover" and cause issues.

Use pilot:setNoDeath() or pilot:setNoDisable() if you want the pilots to either not be destroyed or disabled respectively.

   @tparam {Pilot} pilots Pilots to check when they are all defeated, which is disabled or destroyed.
   @tparam function fn Function to call when all the pilots are defeated.
--]]
function lib.pilots_defeated( pilots, fn )
   local params = {
      pilots      = pilots,
      fn          = fn,
      permdisable = true,
   }
   for k,p in ipairs(pilots) do
      hook.pilot( p, "death", "_hook_pilot_defeated", params )
      hook.pilot( p, "disable", "_hook_pilot_defeated", params )
      hook.pilot( p, "exploded", "_hook_pilot_exploded", params )
   end
end

function _hook_trigger_timer( params )
   local c = params._current
   local p = params[c]

   if type(p[2])=='function' then
      p[2]()
   else
      player.msg( p[2], true ) -- Display on top of the player
      player.autonavReset( 3 )
   end

   params._current = c+1
   if params._current <= #params then
      local t = params[c+1][1]
      -- Allow chaining multiple instantly
      if t <= 0 then
         _hook_trigger_timer( params )
      else
         hook.timer( t, "_hook_trigger_timer", params )
      end
   end
end

--[[--
Can be used to set up a chain of timer events, which will be chained automatically without needing to define additional hook functions.

   @tparam table messages Table of pairs of `{delay, func}` where `delay` is how much to delay the call, and `func` is either a function to call or a message to pass to `player.msg()`.
--]]
function lib.timer_chain( messages )
   messages._current = 1
   hook.timer( messages[1][1], "_hook_trigger_timer", messages )
end

return lib
