--[[--
   Pilot AI helper utilities. Meant for controlling the AI without using explicit pilot#control.
   @module pilotai
--]]
local lanes = require "ai.core.misc.lanes"
local pilotai = {}

--[[
   Makes a pilot try to hyperspace to target while not disabling most functionality.

   Clears current task.

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
   p:taskClear() -- Should run idle and the stuff above
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

      @tparam Pilot p Pilot to make guard.
      @tparam Vec2 pos Position to guard.
--]]
function pilotai.guard( p, pos )
   -- TODO try to figure out how to do this without having to change the AI. Probably a special task could handle it
   p:changeAI( "guard" )
   local m = p:memory()
   m.guardpos = pos
end

--[[
   Tries to clear the system by making all the AI pilots go away. Soft alternative to pilot.clear()

      @tparam[opt=false] boolean allpilots Wether or not to affect all non-player pilots, or just natural pilots.
--]]
function pilotai.clear( allpilots )
   for k,p in ipairs(pilot.get()) do
      local m = p:memory()
      if not p:withPlayer() and (m.natural or allpilots) then
         m.force_leave  = true -- for most AI
         m.loiter       = -1 -- for generic AI
         m.boarded      = 1 -- for pirate AI
         m.doscans      = false
         p:taskClear()
      end
   end
end

return pilotai
