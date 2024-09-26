local fmt = require "format"
local luaspob = require "spob.lua.lib.spob"

luaspob.setup{
   std_land = 50,
   std_bribe = 100,
   msg_granted = {
      function () fmt.f(_([["Welcome, friend {player}. You may dock when ready."]]), {player=player.name()}) end,
   },
   msg_notyet = {
      _([["We can't trust you to land here just yet."]]),
   },
   msg_cantbribe = {
      _([["We have no need for your credits."]]),
   },
}

local _can_land = can_land
function can_land ()
   local ret, msg = _can_land()
   if not ret then
      return ret, msg
   end
   if not faction.get("Thurion"):known() then
      return false, _([["You are not yet authorized to land here."]])
   end
end
