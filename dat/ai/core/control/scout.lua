local atk = require "ai.core.attack.util"

-- Variables
mem.planet_dist = 1500 -- distance to keep from planets
mem.enemy_dist = 800 -- distance to keep from enemies

-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
   local task = ai.taskname()
   local planet

   if task == nil or task == "idle" then
      local enemy = atk.preferred_enemy()

      -- There is an enemy
      if enemy ~= nil then
         if ai.dist(enemy) < mem.enemy_dist or ai.haslockon() then
            ai.pushtask("runaway", enemy)
            return
         end
      end

      -- nothing to do so check if we are too far form the planet (if there is one)
      if mem.approach == nil then
         planet = ai.rndspob()
         if planet ~= nil then
            mem.approach = planet:pos()
         end
      end
      planet = mem.approach

      if planet ~= nil then
         if ai.dist(planet) > mem.planet_dist then
            ai.pushtask("approach")
            return
         end
      end

      -- Go idle if no task
      if task == nil then
         ai.pushtask("idle")
         return
      end

   -- Check if we are near enough
   elseif task == "approach" then
      planet = mem.approach

      if ai.dist( planet ) < mem.planet_dist + ai.minbrakedist() then
         ai.poptask()
         ai.pushtask("idle")
         return
      end

   -- Check if we need to run more
   elseif task == "runaway" then
      local enemy = ai.taskdata()

      if ai.dist(enemy) > mem.enemy_dist and ai.haslockon() == false then
         ai.poptask()
         return
      end
   end
end


-- Required "attacked" function
function attacked ( attacker )
   local task = ai.taskname()

   -- Start running away
   if task ~= "runaway" then
      ai.pushtask("runaway", attacker)

   elseif task == "runaway" then
      if ai.taskdata() ~= attacker then
         -- Runaway from the new guy
         ai.poptask()
         ai.pushtask("runaway", attacker)
      end
   end
end


-- Required "create" function
function create ()
end


-- Effectively does nothing
function idle ()
   if ai.isstopped() == false then
      ai.brake()
   end
end


-- Approaches the target
function approach ()
   local target = mem.approach
   local dist = ai.dist(target)
   ai.face(target)

   -- See if should accel or brake
   if dist > mem.planet_dist then
      ai.accel()
   else
      ai.poptask()
      ai.pushtask("idle")
   end
end
