-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
   task = ai.taskname()

   -- running pirate has healed up some
   if task == "runaway" then
      if ai.parmour() == 100 then
         -- "attack" should be running after "runaway"
         ai.poptask()
      elseif ai.dist( ai.pos( ai.targetid() ) ) > 300 then
         ai.hyperspace()
      end

   -- nothing to do
   elseif task ~= "attack" and task ~= "runaway" then

      -- if getenemy() is 0 then there is no enemy around
      enemy = ai.getenemy()
      if ai.parmour() == 100 and enemy ~= 0 then

         -- taunts!
         num = ai.rnd(0,4)
         if num == 0 then msg = "Prepare to be boarded!"
         elseif num == 1 then msg = "Yohoho!"
         elseif num == 2 then msg = "What's a ship like you doing in a place like this?"
         end
         ai.comm(enemy, msg)

         -- make hostile to the enemy (mainly for player)
         ai.hostile(enemy)

         -- proceed to the attack
         ai.combat() -- set to be fighting
         ai.pushtask(0, "attack", enemy) -- actually begin the attack

      -- nothing to attack
      else
         ai.pushtask(0, "fly")
      end
   end
end

-- Required "attacked" function
function attacked ( attacker )
   task = ai.taskname()

   -- pirate isn't fighting or fleeing already
   if task ~= "attack" and task ~= "runaway" then
      taunt(attacker)
      ai.pushtask(0, "attack", attacker)

   -- pirate is fighting, but switches to new target (doesn't forget the old one though)
   elseif task == "attack" then
      if ai.targetid() ~= attacker then
         ai.pushtask(0, "attack", attacker)
      end
   end
end


function create ()
   if ai.rnd(0,5) ~= 0 then
      ai.setcredits(0, ai.shipprice()/300 )
   end
end


function taunt ( target )
      -- some taunts
      num = ai.rnd(0,4)
      if num == 0 then msg = "You dare attack me!"
      elseif num == 1 then msg = "You think that you can take me on?"
      elseif num == 2 then msg = "Die!"
      elseif num == 3 then msg = "You'll regret this!"
      end
      if msg then ai.comm(target, msg) end
end


-- runs away from the target
function runaway ()
   target = ai.targetid()

   -- make sure target exists
   if not ai.exists(target) then
      ai.poptask()
      return
   end

   dir = ai.face( target, 1 )
   ai.accel()
end

-- attacks the target
function attack ()
   target = ai.targetid()

   -- make sure the pilot target exists
   if not ai.exists(target) then
      ai.poptask()
      return
   end

   dir = ai.face( target )
   dist = ai.dist( ai.pos(target) )

   -- must know when to run away
   if ai.parmour() < 70 then
      ai.pushtask(0, "runaway", target)

   -- should try to hurt the target
   elseif dir < 10 and dist > 300 then
      ai.accel()
   elseif dir < 10 and dist < 300 then
      ai.shoot()
   end

end

-- flies to the player, pointless until hyperspace is implemented
function fly ()
   target = player
   dir = ai.face(target)
   dist = ai.getdist( getpos(target) )
   if dir < 10 and dist > 300 then
      ai.accel()
   end
end

