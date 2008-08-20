include("ai/include/basic.lua")

-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
   local task = ai.taskname()
   local enemy = ai.getenemy()

   -- Think for attacking
   if task == "attack" then
      attack_think()

   -- Enemy sighted
   elseif enemy ~= nil then
      ai.pushtask(0, "attack", enemy)

   -- Enter hyperspace if possible
   elseif task == "hyperspace" then 
      ai.hyperspace() -- try to hyperspace 

   -- Get new task
   else
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
   if rnd.int(0,2)==0 then -- money less often, but more
      ai.setcredits( rnd.int(1000, ai.shipprice()/70) )
   end
   if rnd.int(0,2)==0 then
      ai.broadcast("The Empire is watching you.")
   end
end

-- taunts
function taunt ( target )
   taunts = {
         "You dare attack me!",
         "You are no match for the Empire!",
         "The Empire will have your head!",
         "You'll regret this!"
   }
   ai.comm(target, taunts[ rnd.int(1,#taunts) ])
end

