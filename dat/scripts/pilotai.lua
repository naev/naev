local lanes = require "ai.core.misc.lanes"

local pilotai = {}

--[[
   Makes a pilot try to hyperspace to target while not disabling most functionality.

      @tparam Pilot p Pilot to command.
      @tparam Jump target Target jump point.
--]]
function pilotai.hyperspace( p, target )
   local m = p:memory()
   m.loiter = 0
   m.goal = "hyperspace"
   m.goal_hyperspace = target
   m.goal_pos = target:pos()
   m.route = lanes.getRouteP( p, m.goal_pos )
end

return pilotai
