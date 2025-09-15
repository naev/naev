--[[

   Library that supersedes "proximity" trying to rely less on globals and keep
   a more simple and less flexible API.

   @madule trigger
--]]
local lib = {}

local _heartbeat_distance_params
function _heartbeat_player_distance()
   local p = _heartbeat_distance_params
   if player.pos():dist2( p.target ) <= p.dist2 then
      p.fn()
   else
      hook.timer( 1, "_heartbeat_player_distance" )
   end
end

--[[--
Runs a function 'fn' when the player is within a distance of a target position.
--]]
function lib.distance_player( target, distance, fn )
   _heartbeat_distance_params = {
      target   = target,
      fn       = fn,
      dist2    = distance^2,
   }
   hook.timer( 1, "_heartbeat_player_distance" )
end

return lib
