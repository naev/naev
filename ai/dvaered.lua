include("ai/include/basic.lua")

-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
   task = ai.taskname()

   enemy = ai.getenemy()
   if task ~= "attack" and enemy ~= nil then
      ai.hostile(enemy)
      ai.pushtask(0, "attack", enemy)

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
   task = ai.taskname()
   if task ~= "attack" and task ~= "runaway" then

      -- some taunting
      taunt( attacker )

      -- now pilot fights back
      ai.pushtask(0, "attack", attacker)

   elseif task == "attack" then
      if ai.targetid() ~= attacker then
         ai.pushtask(0, "attack", attacker)
      end
   end
end

function create ()
   ai.setcredits( rnd.int(1000, ai.shipprice()/100) )
end

-- taunts
function taunt ( target )
   taunts = {
         "Prepare to face annihilation!",
         "I shall wash my hull in your blood!",
         "Your head will make a great trophy!"
   }
   ai.comm( target, taunts[ rnd.int(1,#taunts) ] )
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
      ai.settimer(0, rnd.int(8000,15000))
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


