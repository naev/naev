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
   elseif ai.taskname() == "none" then
      ai.pushtask(0, "scan", ai.rndpilot())
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
   ai.setcredits( rnd.int(1000, ai.shipprice()/200) )
   if rnd.int(0,2)==0 then
      ai.broadcast("This area is under militia survellance.")
   end
end

-- taunts
function taunt ( target )
   taunts = {
         "You dare attack me!",
         "You think that you can take me on?",
         "Die!",
         "You'll regret this!"
   }
   ai.comm(target, taunts[ rnd.int(1,#taunts) ])
end

-- flies to the player
function scan ()
   target = ai.targetid()
   if not ai.exists(target) then
      ai.poptask()
      return
   end
   dir = ai.face(target)
   dist = ai.dist( ai.pos(target) )
   if dir < 10 and dist > 300 then
      ai.accel()
   elseif dist < 300 then -- scan the target
      ai.poptask()
   end
end

