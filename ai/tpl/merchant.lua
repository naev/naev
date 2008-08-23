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
      if ai.dist( ai.pos( ai.targetid() ) ) > 400 then
         ai.hyperspace()
      end

   -- Find something to do
   elseif task == "none" then
      planet = ai.landplanet()
      -- planet must exist
      if planet == nil then
         ai.pushtask(0, "hyperspace")
      else
         ai.pushtask(0, "hyperspace")
         ai.pushtask(0, "land", planet)
      end
   end
end

function sos ()
end

-- Required "attacked" function
function attacked ( attacker )
   if ai.taskname() ~= "runaway" then
      sos()
      -- Sir Robin bravely ran away
      ai.pushtask(0, "runaway", attacker)
   else -- run away from the new baddie
      ai.poptask()
      ai.pushtask(0, "runaway", attacker)
   end
end

function create ()
end

