include("ai/include/basic.lua")

-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
   task = ai.taskname()
   enemy = ai.getenemy()

   -- Runaway if enemy is near
   if task ~= "runaway" and enemy ~= nil and ai.dist(enemy) < 500 then
      ai.poptask()
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
         ai.pushtask(0, "goto", planet)
      end
   end
end

-- Required "attacked" function
function attacked ( attacker )
   if ai.taskname() ~= "runaway" then

      -- some messages
      num = rnd.int(0,3)
      if num == 0 then msg = "Mayday! We are under attack!"
      elseif num == 1 then msg = "Requesting assistance.  We are under attack!"
      elseif num == 2 then msg = "Merchant vessle here under attack! Help!"
      end
      if msg then ai.broadcast(msg) end

      -- Sir Robin bravely ran away
      ai.pushtask(0, "runaway", attacker)
   else -- run away from the new baddie
      ai.poptask()
      ai.pushtask(0, "runaway", attacker)
   end
end

-- Gives the pilot it's initial stuff
function create ()
   ai.setcredits( rnd.int(200, ai.shipprice()/100) )

   num = rnd.int(12)
   if num < 5 then
      cargo = "Food"
   elseif num < 8 then
      cargo = "Ore"
   elseif num < 10 then
      cargo = "Industrial Goods"
   elseif num < 12 then
      cargo = "Luxury Goods"
   else
      cargo = "Medicine"
   end
   ai.setcargo( cargo, rnd.int(0, ai.cargofree() ) )
end

-- flies to the target
function goto ()
   target = ai.target()
   dir = ai.face(target)
   dist = ai.dist( target )
   bdist = ai.minbrakedist()
   if dir < 10 and dist > bdist then
      ai.accel()
   elseif dir < 10 and dist < bdist then
      ai.poptask()
      ai.pushtask(0,"stop")
   end
end

-- brakes
function stop ()
   if ai.isstopped() then
      ai.stop()
      ai.poptask()
      ai.settimer(0, rnd.int(8000,15000)) -- We wait duringa while
      ai.pushtask(0,"land")
   else
      ai.brake()
   end
end

-- waits
function land ()
   if ai.timeup(0) then
      ai.pushtask(0,"hyperspace")
   end
end
