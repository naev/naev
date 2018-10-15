include("dat/ai/include/basic.lua")

-- Variables
mem.enemy_close = 500 -- Distance enemy is too close for comfort

-- Required control rate
control_rate    = 2

-- Required "control" function
function control ()
   task  = ai.taskname()
   enemy = ai.getenemy()

   -- Runaway if enemy is near
   if task ~= "runaway" and enemy ~= nil and
         (ai.dist(enemy) < mem.enemy_close or ai.haslockon()) then
      if task ~= nil then
         ai.poptask()
      end
      ai.pushtask("runaway",enemy)

   -- Try to jump when far enough away
   elseif task == "runaway" then
      target = ai.target()

      -- Check if should still run.
      if not target:exists() then
         ai.poptask()
         return
      end

      dist = ai.dist( target )

      if mem.attacked then
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
      end

      -- See if another enemy is closer
      if enemy ~= nil and enemy ~= target then
         ai.poptask()
         ai.pushtask("runaway",enemy)
      end

      -- Try to jump.
      if dist > 400 then
         ai.hyperspace()
      end

   -- Find something to do
   elseif task == nil then
      planet = ai.landplanet()
      -- planet must exist
      if planet == nil then
         ai.settimer(0, rnd.int(1000, 3000))
         ai.pushtask("enterdelay")
      else
         mem.land = planet:pos()
         ai.pushtask("hyperspace")
         ai.pushtask("land")
      end
   end
end

   -- Delays the ship when entering systems so that it doesn't leave right away
function enterdelay ()
   if ai.timeup(0) then
      ai.pushtask("hyperspace")
   end
end


function sos ()
end

-- Required "attacked" function
function attacked ( attacker )
   mem.attacked = true

   if ai.taskname() ~= "runaway" then
      -- Sir Robin bravely ran away
      ai.pushtask("runaway", attacker)
   else -- run away from the new baddie
      ai.poptask()
      ai.pushtask("runaway", attacker)
   end
end

function create ()
end

