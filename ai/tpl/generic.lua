include("ai/include/basic.lua")
include("ai/include/attack.lua")

--[[
-- Variables to adjust AI
--
-- These variables can be used to adjust the generic AI to suit other roles.
--]]
mem.armour_run     = 0 -- At which damage to run at
mem.armour_return  = 0 -- At which armour to return to combat
mem.shield_run     = 0 -- At which shield to run
mem.shield_return  = 0 -- At which shield to return to combat
mem.aggressive     = false -- Should pilot actively attack enemies?
mem.safe_distance  = 300 -- Safe distance from enemies to jump
mem.land_planet    = true -- Should land on planets?
mem.distressrate   = 3 -- Number of ticks before calling for help
mem.distressmsg    = nil -- Message when calling for help


-- Required control rate
control_rate   = 2

-- Required "control" function
function control ()
   task = ai.taskname()
   enemy = ai.getenemy()

   -- Get new task
   if task == "none" then
      -- We'll first check enemy.
      if enemy ~= nil and mem.aggressive then
         taunt(enemy, true)
         ai.pushtask(0, "attack", enemy)
      else
         idle()
      end

   -- Don't stop boarding
   elseif task == "board" or task == "boardstop" then
      -- We want to think in case another attacker gets close
      attack_think()

   -- Think for attacking
   elseif task == "attack" then
      target = ai.target()

      -- Needs to have a target
      if not ai.exists(target) then
         ai.poptask()
         return
      end

      -- Runaway if needed
      if (mem.shield_run > 0 and ai.pshield() < mem.shield_run
               and ai.pshield() < ai.pshield(target) ) or
            (mem.armour_run > 0 and ai.parmour() < mem.armour_run
               and ai.parmour() < ai.parmour(target) ) then
         ai.pushtask(0, "runaway", target)

      -- Think like normal
      else
         attack_think()
      end

      -- Handle distress
      gen_distress()

   -- Pilot is running away
   elseif task == "runaway" then
      target = ai.target()

      -- Needs to have a target
      if not ai.exists(target) then
         ai.poptask()
         return
      end

      dist = ai.dist( target )

      -- Should return to combat?
      if mem.aggressive and ((mem.shield_return > 0 and ai.pshield() >= mem.shield_return) or
            (mem.armour_return > 0 and ai.parmour() >= mem.armour_return)) then
         ai.poptask() -- "attack" should be above "runaway"

      -- Try to jump
      elseif dist > mem.safe_distance then
         ai.hyperspace()
      end

      -- Handle distress
      gen_distress()

   -- Enemy sighted, handled after running away
   elseif enemy ~= nil and mem.aggressive then
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

   -- Notify that pilot has been attacked before
   mem.attacked = true

   if task ~= "attack" and task ~= "runaway" then

      -- some taunting
      taunt( attacker, false )

      -- now pilot fights back
      ai.pushtask(0, "attack", attacker)

   -- Let attacker profile handle it.
   elseif task == "attack" then
      attack_attacked( attacker )

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
   if planet == nil or mem.land_planet == false then
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

   -- Make suree target exists
   if not ai.exists( attacker ) then
      ai.poptask()
      return
   end

   -- Must be aggressive
   if not mem.aggressive then
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

      if not ai.exists(target) or ai.dist(target) > ai.dist(t) then
         ai.pushtask( 0, "attack", t )
      end
   end
end


-- Handles generating distress messages
function gen_distress ( target )

   -- Must have a valid distress rate
   if mem.distressrate <= 0 then
      return
   end

   -- Only generate distress if have been attacked before
   if not mem.attacked then
      return
   end

   -- Update distres counter
   if mem.distressed == nil then
      mem.distressed = 1
   else
      mem.distressed = mem.distressed + 1
   end

   -- See if it's time to trigger distress
   if mem.distressed > mem.distressrate then
      ai.distress( mem.distressmsg )
      mem.distressed = 1
   end

end

