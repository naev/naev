-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
   task = ai.taskname()
   if task == "hyperspace" then
      ai.hyperspace() -- try to hyperspace
   elseif task == "runaway" then
      if ai.dist( ai.pos( ai.targetid() ) ) > 300 then
         ai.hyperspace()
      end
   elseif task == "none" then
      planet = ai.rndplanet()
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
      num = ai.rnd(0,3)
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
   ai.setcredits( ai.rnd(200, ai.shipprice()/100) )

   num = ai.rnd(0,1)
   if num == 0 then
      cargo = "Food"
   elseif num ==1 then
      cargo = "Ore"
   end
   ai.setcargo( cargo, ai.rnd(0, ai.cargofree() ) )
end

-- runs away
function runaway ()
   target = ai.targetid()

   if not ai.exists(target) then
      ai.pushtask()
      ai.pushtask(0,"hyperspace")
      return
   end

   dir = ai.face( target, 1 )
   ai.accel()
   if ai.hasturrets() then
      dist = ai.dist( ai.pos(target) )
      if dist < 300 then
         ai.settarget(target)
         ai.shoot()
      end
   end
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
      ai.settimer(0, ai.rnd(8000,15000))
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

-- goes hyperspace
function hyperspace ()
   dir = ai.face(-1) -- face away from (0,0)
   if (dir < 10) then
      ai.accel()
   end
end

