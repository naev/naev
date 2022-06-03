--[[--
   Pilot AI helper utilities. Meant for controlling the AI without using explicit pilot#control.
   @module pilotai
--]]
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

--[[
   Makes a pilot patrol a set of waypoints indefinately

      @tparam Pilot p Pilot to make patrol.
      @tparam Table waypoints Ordered table with the waypoints as Vec2.
--]]
function pilotai.patrol( p, waypoints )
   local m = p:memory()
   m.waypoints = waypoints
   m.loiter = math.huge
end

return pilotai
