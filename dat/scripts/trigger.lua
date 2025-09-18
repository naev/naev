--[[
   Library that supersedes "proximity" trying to rely less on globals and keep
   a more simple and less flexible API.

   Hooks in this library are not persistent across saves and meant for
   in-system events (usually triggered from a hook.enter() hook).

   @madule trigger
--]]
local lib = {}

function _hook_player_distance( params )
   if player.pos():dist2( params.target ) <= params.dist2 then
      params.fn()
   else
      hook.timer( 1, "_hook_player_distance", params )
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
   hook.timer( 1, "_hook_player_distance", params )
end

function _hook_pilot_defeated( plt, _killer, params )
   -- Make disable permanent if necessary
   if params.permdisable and plt:disabled() then
      plt:setDisable()
   end

   -- Remove pilot from list of pilots
   local newplts = {}
   for p in ipairs(params.pilots) do
      if p~=plt then
         table.insert( newplts, p )
      end
   end
   params.pilots = newplts

   -- Run hook if necessary
   if #params.pilots <= 0 then
      params.fn()
   end
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
      pilots   = pilots,
      fn       = fn,
      permdisable = true,
   }
   for p in ipairs(pilots) do
      hook.pilot( p, "death", "_hook_pilot_defeated", params )
      hook.pilot( p, "disable", "_hook_pilot_defeated", params )
   end
end

return lib
