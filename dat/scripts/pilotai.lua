--[[--
   Pilot AI helper utilities. Meant for controlling the AI without using explicit pilot#control.
   @module pilotai
--]]
local lanes = require "ai.core.misc.lanes"
local pilotai = {}

--[[--
   Applies a function to one or more pilots.

      @tparam Pilot|table plts Pilots to apply command to.
      @tparam function func Function to apply to each pilot. Should take a single parameter.
--]]
function pilotai.apply( plts, func )
   if type(plts)~="table" then
      plts = {plts}
   end
   for k,p in ipairs(plts) do
      func(p)
   end
end

--[[--
   Makes a pilot try to hyperspace to target while not disabling most functionality.

   Clears current task.

      @tparam Pilot|table plts Pilot or pilots to command.
      @tparam Jump|nil target Target jump point. If nil, tries to find a random hyperspace.
--]]
function pilotai.hyperspace( plts, target )
   if type(plts)~="table" then
      plts = {plts}
   end
   if not target then
      local jmps = {}
      -- We assume that they all have the same faction as the first...
      local usehidden = plts[1]:faction():usesHiddenJumps()
      for k,v in ipairs(system.cur():jumps(true)) do
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
   for k,p in ipairs(plts) do
      local m = p:memory()
      m.loiter = 0
      m.goal = "hyperspace"
      m.goal_hyperspace = target
      m.goal_pos = target:pos()
      m.route = lanes.getRouteP( p, m.goal_pos )
      p:taskClear() -- Should run idle and the stuff above
   end
end

--[[--
   Makes a pilot patrol a set of waypoints indefinitely

      @tparam Pilot|table plts Pilot or pilots to make patrol.
      @tparam Table waypoints Ordered table with the waypoints as Vec2.
--]]
function pilotai.patrol( plts, waypoints )
   pilotai.apply( plts, function( p )
      local m = p:memory()
      m.waypoints = waypoints
      m.loiter = math.huge
   end )
end

--[[--
   Makes the pilot go to a certain position and guard it. Note that this changes the AI of the pilot.

      @tparam Pilot|table plts Pilot or pilots to make guard.
      @tparam Vec2 pos Position to guard.
--]]
function pilotai.guard( plts, pos )
   pilotai.apply( plts, function( p )
      -- TODO try to figure out how to do this without having to change the AI. Probably a special task could handle it
      p:changeAI( "guard" )
      local m = p:memory()
      m.guardpos = pos
   end )
end

--[[--
   Tries to clear the system by making all the AI pilots go away. Soft alternative to pilot.clear()

      @tparam[opt=false] boolean allpilots Whether or not to affect all non-player pilots, or just natural pilots.
--]]
function pilotai.clear( allpilots )
   for k,p in ipairs(pilot.get()) do
      local m = p:memory()
      if not p:withPlayer() and (m.natural or allpilots) then
         m.force_leave  = true -- for most AI
         m.loiter       = -1 -- for generic AI
         m.boarded      = 1 -- for pirate AI
         m.doscans      = false
         m.vulnerability = math.huge -- Have them be targeted less
         p:taskClear()

         -- Try to make them leave from the closest place
         local pp = p:pos()
         local pfct = p:faction()
         local dist = math.huge
         local distj = math.huge
         local candspob, candjump, closestjump
         for i,s in ipairs(system.cur():spobs()) do
            local sfct = s:faction()
            if s:services().land and sfct and not pfct:areEnemies(sfct) then
               local d = s:pos():dist2( pp )
               if d < dist then
                  candspob = s
                  dist = d
               end
            end
         end
         for i,j in ipairs(system.cur():jumps(true)) do
            local jfct = j:dest():faction()
            if jfct and not pfct:areEnemies(jfct) then
               local d = j:pos():dist2( pp )
               if d < dist then
                  candspob = nil
                  candjump = j
                  dist = d
               end
               if d < distj then
                  closestjump = j
                  distj = d
               end
            end
         end

         -- Try to get them out
         if candspob then
            m.goal = "planet"
            m.goal_planet = candspob
            m.goal_pos = candspob:pos()
            p:pushtask("land", m.goal_planet)
         elseif candjump then
            m.goal = "hyperspace"
            m.goal_hyperspace = candjump
            m.goal_pos = candjump:pos()
            p:pushtask("hyperspace",m.goal_hyperspace)
         elseif closestjump then
            -- Get them out the closest jump
            candjump = closestjump
            m.goal = "hyperspace"
            m.goal_hyperspace = candjump
            m.goal_pos = candjump:pos()
            p:pushtask("hyperspace",m.goal_hyperspace)
         end
      end
   end
end

return pilotai
