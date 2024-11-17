local fmt = require "format"
local luaspob = require "spob.lua.lib.spob"

luaspob.setup{
   std_land = 0,
   std_bribe = 101,
   msg_granted = {
      function () return player.name() and fmt.f(_([["Welcome, {player}. You may dock when ready."]]), {player=player.name()}) or "" end,
      _([["Dock at the life-support enabled ports when ready."]]),
   },
   msg_denied = {
      _([["We lack trust in you to allow you to land here."]]),
      _([["You are not welcome among the Thurion."]]),
   },
   msg_cantbribe = {
      _([["The Thurion have no need for your vile credits."]]),
      _([["Digital beings have no need of your vile currency."]]),
      _([["Your vile credits will not sway our digital will."]]),
   },
}

local _can_land = can_land
function can_land ()
   local ret, msg = _can_land()
   if type(msg) == "function" then
      msg = msg()
   end
   if not ret then
      return ret, msg
   end
   if not faction.get("Thurion"):known() then
      return false, _([["You are not yet authorized to land here."]])
   end
   return ret, msg
end
