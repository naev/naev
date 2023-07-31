local lanes = require 'ai.core.misc.lanes'
local scans = require 'ai.core.misc.scans'
local atk = require "ai.core.attack.util"

-- Default task to run when idle
function idle ()
   local p = ai.pilot()
   -- Aggressives will try to find enemies first, before falling back on
   -- loitering, to avoid weird stuff starting to scan before attacking
   if not mem.force_leave and mem.aggressive then
      local enemy  = atk.preferred_enemy( nil, true )
      if should_attack( enemy ) then
         ai.pushtask( "attack", enemy )
         return
      end
   end

   -- Try to leave
   if mem.force_leave or mem.loiter == 0 then
      -- Get a goal
      if not mem.goal then
         if mem.land_planet and not mem.tookoff then
            local planet = ai.landspob( mem.land_friendly )
            if planet ~= nil then
               mem.goal = "planet"
               mem.goal_planet = planet
               mem.goal_pos = planet:pos()
            end
         end
         if not mem.goal then
            local hyperspace = ai.rndhyptarget()
            if hyperspace then
               mem.goal = "hyperspace"
               mem.goal_hyperspace = hyperspace
               mem.goal_pos = hyperspace:pos()
            end
         end
         -- Wasn't able to find a goal, so just loiter more
         if not mem.goal then
            mem.loiter = 1
            return
         end

         mem.route = lanes.getRouteP( p, mem.goal_pos )
      end

      -- Arrived at goal
      if #mem.route == 0 then
         if mem.goal == "planet" then
            ai.pushtask("land", mem.goal_planet)
         elseif mem.goal == "hyperspace" then
            ai.pushtask("hyperspace", mem.goal_hyperspace)
         end
         return
      end

      -- Continue to goal
      local pos = mem.route[1]
      table.remove( mem.route, 1 )
      if #mem.route==1 then
         ai.pushtask("loiter_last", pos + vec2.newP(200*rnd.rnd(), rnd.angle()) )
      else
         ai.pushtask("loiter", pos + vec2.newP(200*rnd.rnd(), rnd.angle()) )
      end
      return

   else -- Stay. Have a beer.
      if mem.doscans then
         local target = scans.get_target()
         if target then
            -- Don't scan if they're going to be attacked anyway
            if ai.isenemy(target) then
               -- TODO probably use should_attack here
               ai.pushtask( "attack", target )
            else
               scans.push( target )
            end
            return
         end
      end

      -- Check to see if we want to patrol waypoints
      -- TODO should waypoints use the mem.loiter also?
      if mem.waypoints then
         -- If haven't started patroling, find the closest waypoint
         if not mem._waypoint_cur then
            local dist = math.huge
            local closest = nil
            for k,v in pairs(mem.waypoints) do
               local vd = ai.dist( v )
               if vd < dist then
                  dist = vd
                  closest = k
               end
            end
            mem._waypoint_cur = closest
         else
            mem._waypoint_cur = math.fmod( mem._waypoint_cur, #mem.waypoints )+1
         end
         -- Go to the next position
         ai.pushtask( "loiter", mem.waypoints[ mem._waypoint_cur ] )
      else
         -- Go to an interesting
         if not mem.route then
            local target = lanes.getPointInterestP( p )
            mem.route = lanes.getRouteP( p, target )
         end
         local pos = mem.route[1]
         table.remove( mem.route, 1 )
         if #mem.route == 0 then
            mem.loiter = mem.loiter - 1
            mem.route = nil
         end
         ai.pushtask("loiter", pos + vec2.newP(200*rnd.rnd(), rnd.angle()) )
      end
   end
end

-- Settings
mem.waypoints     = nil
mem.land_friendly = false -- Land on only friendly by default
mem.doscans       = false
mem.loiter        = 10
