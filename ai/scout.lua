include("ai/include/basic.lua")

-- Some vars
planet_dist = 1000 -- distance to keep from planets
enemy_dist = 700 -- distance to keep from enemies

-- Required control rate
control_rate = 2
-- Required "control" function
function control ()
   task = ai.taskname()

   if task == "none" or task == "idle" then
      enemy = ai.getenemy()

      -- There is an enemy
      if enemy ~= 0 then
         -- make hostile to the enemy (mainly for player)
         
         if ai.dist(enemy) < enemy_dist or ai.haslockon() then
            ai.pushtask(0, "runaway", enemy)
            return
         end
      end

      -- nothing to do so check if we are too far form the planet (if there is one)
      planet = ai.rndplanet()

      if planet ~= nil then
         if ai.dist(planet) > planet_dist then
            ai.pushtask(0, "approach", planet)
            return
         end
      end

      -- Go idle if no task
      if task == "none" then
         ai.pushtask(0, "idle")
         return
      end

   -- Check if we are near enough
   elseif task == "approach" then
      planet = ai.target()
      
      if ai.dist( planet ) < planet_dist + ai.minbrakedist() then
         ai.poptask()
         ai.pushtask(0, "idle")
         return
      end

   -- Check if we need to run more
   elseif task == "runaway" then
      enemy = ai.targetid()

      if ai.dist(enemy) > enemy_dist and ai.haslockon() == false then
         ai.poptask()
         return
      end
   end
end


-- Required "attacked" function
function attacked ( attacker )
   task = ai.taskname()
   if task ~= "runaway" then

      ai.pushtask(0, "runaway", attacker)

   elseif task == "runaway" then
      if ai.targetid() ~= attacker then
         ai.pushtask(0, "runaway", attacker)
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
   target = ai.target()
   dir = ai.face(target)
   ai.accel()
end


