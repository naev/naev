--[[--
   Pilot AI helper utilities. Meant for controlling the AI without using explicit pilot#control.
   @module pilotai
--]]
local lanes = require "ai.core.misc.lanes"
local pilotai = {}

--[[
   Makes a pilot try to hyperspace to target while not disabling most functionality.

      @tparam Pilot p Pilot to command.
      @tparam Jump|nil target Target jump point. If nil, tries to find a random hyperspace.
--]]
function pilotai.hyperspace( p, target )
   if not target then
      local jmps = {}
      local usehidden = p:faction():usesHiddenJumps()
      for k,v in ipairs(system.cur():jumps()) do
         if usehidden or not v:hidden() then
            table.insert( jmps, v )
         end
      end
      if #jmps==0 then
         warn(_("Trying to run 'pilotai.hyperspace' without a target in a system without valid jump points!"))
         return
      end
      target = jmps[ rnd.rnd(1,#jmps) ]
   end

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

--[[
   Makes the pilot go to a certain position and guard it. Note that this changes the AI of the pilot.

      @tparam p Pilot to make guard.
      @tparam Vec2 Position to guard.
--]]
function pilotai.guard( p, pos )
   -- TODO try to figure out how to do this without having to change the AI. Probably a special task could handle it
   p:changeAI( "guard" )
   local m = p:memory()
   m.guardpos = pos
end

return pilotai
