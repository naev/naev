include("ai/include/basic.lua")

-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
   task = ai.taskname()

   -- Handle updating enemies
   if task == "attack" then
      if ai.parmour() < 80 then
         ai.pushtask(0, "runaway", ai.targetid())
      else
         attack_think()
      end

   elseif task == "hyperspace" then
      ai.hyperspace() -- Try to hyperspace

   -- running pirate has healed up some
   elseif task == "runaway" then
      if ai.parmour() == 100 then
         -- "attack" should be running after "runaway"
         ai.poptask()
      elseif ai.dist( ai.pos( ai.targetid() ) ) > 300 then
         ai.hyperspace()
      end

   -- nothing to do
   else

      -- if getenemy() is 0 then there is no enemy around
      enemy = ai.getenemy()
      if ai.parmour() == 100 and enemy ~= 0 then
         taunt( enemy, true )
         ai.pushtask(0, "attack", enemy) -- actually begin the attack

      -- nothing to attack
      else
         ai.pushtask(0, "hyperspace")
      end
   end
end

-- Required "attacked" function
function attacked ( attacker )
   task = ai.taskname()

   -- pirate isn't fighting or fleeing already
   if task ~= "attack" and task ~= "runaway" then
      taunt(attacker, false)
      ai.pushtask(0, "attack", attacker)

   -- pirate is fighting, but switches to new target (doesn't forget the old one though)
   elseif task == "attack" then
      if ai.targetid() ~= attacker then
         ai.pushtask(0, "attack", attacker)
      end
   end
end


function create ()
   if rnd.int(0,5) ~= 0 then
      ai.setcredits(0, ai.shipprice()/100 )
   end
end


function taunt ( target, offense )

   -- Only 50% of actually taunting.
   if rnd.int(0,1) == 0 then
      return
   end

   -- some taunts
   if offense then
      taunts = {
            "Prepare to be boarded!",
            "Yohoho!",
            "What's a ship like you doing in a place like this?"
      }
   else
      taunts = {
            "You dare attack me!",
            "You think that you can take me on?",
            "Die!",
            "You'll regret this!"
      }
   end

   ai.comm(target, taunts[ rnd.int(1,#taunts) ])
end

