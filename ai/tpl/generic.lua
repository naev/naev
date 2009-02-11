include("ai/include/basic.lua")
include("ai/include/attack.lua")

--[[
-- Variables to adjust AI
--
-- These variables can be used to adjust the generic AI to suit other roles.
--]]
armour_run = 0 -- At which damage to run at
armour_return = 0 -- At which armour to return to combat
shield_run = 0 -- At which shield to run
shield_return = 0 -- At which shield to return to combat
aggressive = false -- Should pilot actively attack enemies?
safe_distance = 300 -- Safe distance from enemies to jump
land_planet = true -- Should land on planets?


-- Required control rate
control_rate = 2

-- Required "control" function
function control ()
   task = ai.taskname()
   enemy = ai.getenemy()

   -- Get new task
   if task == "none" then
      -- We'll first check enemy.
      if enemy ~= nil and aggressive then
         taunt(enemy, true)
         ai.pushtask(0, "attack", enemy)
      else
         idle()
      end

   -- Think for attacking
   elseif task == "attack" then
      target = ai.target()

      -- Needs to have a target
      if target == nil then
         ai.poptask()
      end

      -- Runaway if needed
      if (shield_run > 0 and ai.pshield() < shield_run
               and ai.pshield() < ai.pshield(target) ) or
            (armour_run > 0 and ai.parmour() < armour_run
               and ai.parmour() < ai.parmour(target) ) then
         ai.pushtask(0, "runaway", target)

      -- Think like normal
      else
         attack_think()
      end

   -- Pilot is running away
   elseif task == "runaway" then
      target = ai.target()
      dist = ai.dist( target )

      -- Should return to combat?
      if aggressive and ((shield_return > 0 and ai.pshield() >= shield_return) or
            (armour_return > 0 and ai.parmour() >= armour_return)) then
         ai.poptask() -- "attack" should be above "runaway"

      -- Try to jump
      elseif dist > safe_distance then
         ai.hyperspace()
      end

   -- Enemy sighted, handled after running away
   elseif enemy ~= nil and aggressive then
      taunt(enemy, true)
      ai.pushtask(0, "attack", enemy)

   -- Enter hyperspace if possible
   elseif task == "hyperspace" then 
      ai.hyperspace() -- try to hyperspace 

   end
end

-- Required "attacked" function
function attacked ( attacker )
   task = ai.taskname()
   target = ai.target()

   if task ~= "attack" and task ~= "runaway" then

      -- some taunting
      taunt( attacker, false )

      -- now pilot fights back
      ai.pushtask(0, "attack", attacker)

   elseif task == "attack" then
      if ai.target() ~= attacker and 
            ai.dist(attacker) < ai.dist(target) then
         ai.pushtask(0, "attack", attacker)
      end
   elseif task == "runaway" then
      if ai.target() ~= attacker then
         ai.poptask()
         ai.pushtask(0, "runaway", attacker)
      end
   end
end

-- Default task to run when idle
function idle ()
   planet = ai.landplanet()
   -- planet must exist
   if planet == nil or land_planet == false then
      ai.settimer(0, rnd.int(1000, 3000))
      ai.pushtask(0, "enterdelay")
   else
      mem.land = planet
      ai.pushtask(0, "hyperspace")
      ai.pushtask(0, "land")
   end
end

-- Delays the ship when entering systems so that it doesn't leave right away
function enterdelay ()
   if ai.timeup(0) then
      ai.pushtask(0, "hyperspace")
   end
end

function create ()
   attack_choose()
end

-- taunts
function taunt ( target, offensive )
   -- Empty stub
end


-- Handle distress signals
function distress ( pilot, attacker )

   -- Must be aggressive
   if not aggressive then
      return
   end

   -- Make sure pilot is setting his target properly
   if pilot == attacker then
      return
   end

   -- If ally, engage offender
   if ai.isally(pilot) then
      t = attacker
   -- If enemy, engage distressed pilot
   elseif ai.isenemy(pilot) then
      t = pilot
   -- Else ignore
   else
      return
   end

   task = ai.taskname()
   -- If not attacking nor fleeing, begin attacking
   if task ~= "attack" and task ~= "runaway" then
      ai.pushtask( 0, "attack", t )
   -- We're sort of busy
   elseif task == "attack" then
      target = ai.target()

      if ai.dist(target) > ai.dist(t) then
         ai.pushtask( 0, "attack", t )
      end
   end
end

