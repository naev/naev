include("ai/include/basic.lua")

-- Variables
enemy_close = 500 -- Distance enemy is too close for comfort

-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
   task = ai.taskname()
   enemy = ai.getenemy()

   -- Runaway if enemy is near
   if task ~= "runaway" and enemy ~= nil and ai.dist(enemy) < enemy_close then
      if task ~= "none" then
         ai.poptask()
      end
      ai.pushtask(0,"runaway",enemy)

   -- Enter hyperspace if possible
   elseif task == "hyperspace" then
      ai.hyperspace() -- try to hyperspace

   -- Try to jump when far enough away
   elseif task == "runaway" then
      target = ai.target()

      -- Check if should still run.
      if not ai.exists(target) then
         ai.poptask()
         return
      end

      dist = ai.dist( target )

      -- Increment distress
      if mem.distressed == nil then
         mem.distressed = 10 -- Distresses more quickly at first
      else
         mem.distressed = mem.distressed + 1
      end

      -- Check to see if should send distress signal.
      if mem.distressed > 10 then
         sos()
         mem.distressed = 1
      end

      -- See if another enemy is closer
      if enemy ~= nil and enemy ~= target then
         ai.poptask()
         ai.pushtask(0,"runaway",enemy)
      end

      -- Try to jump.
      if dist > 400 then
         ai.hyperspace()
      end

   -- Find something to do
   elseif task == "none" then
      planet = ai.landplanet()
      -- planet must exist
      if planet == nil then
         ai.settimer(0, rnd.int(1000, 3000))
         ai.pushtask(0, "enterdelay")
      else
         mem.land = planet
         ai.pushtask(0, "hyperspace")
         ai.pushtask(0, "land")
      end
   end
end

   -- Delays the ship when entering systems so that it doesn't leave right away
function enterdelay ()
   if ai.timeup(0) then
      ai.pushtask(0, "hyperspace")
   end
end


function sos ()
end

-- Required "attacked" function
function attacked ( attacker )
   if ai.taskname() ~= "runaway" then
      -- Sir Robin bravely ran away
      ai.pushtask(0, "runaway", attacker)
   else -- run away from the new baddie
      ai.poptask()
      ai.pushtask(0, "runaway", attacker)
   end
end

function create ()
end

