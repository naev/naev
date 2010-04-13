include("ai/include/basic.lua")
include("ai/include/attack.lua")

--[[
-- Variables to adjust AI
--
-- These variables can be used to adjust the generic AI to suit other roles.
--]]
mem.enemyclose     = nil -- Distance at which an enemy is considered close
mem.armour_run     = 0 -- At which damage to run at
mem.armour_return  = 0 -- At which armour to return to combat
mem.shield_run     = 0 -- At which shield to run
mem.shield_return  = 0 -- At which shield to return to combat
mem.aggressive     = false -- Should pilot actively attack enemies?
mem.defensive      = true -- Should pilot defend itself
mem.safe_distance  = 300 -- Safe distance from enemies to jump
mem.land_planet    = true -- Should land on planets?
mem.distress       = true -- AI distresses
mem.distressrate   = 3 -- Number of ticks before calling for help
mem.distressmsg    = nil -- Message when calling for help
mem.distressmsgfunc = nil -- Function to call when distressing


-- Required control rate
control_rate   = 2

-- Required "control" function
function control ()
   local task = ai.taskname()
   local enemy = ai.getenemy()

   -- Reset distress if not fighting/running
   if task ~= "attack" and task ~= "runaway" then
      mem.attacked = nil
   end

   -- Get new task
   if task == "none" then
      local attack = false

      -- We'll first check enemy.
      if enemy ~= nil and mem.aggressive then
         -- Check if we have minimum range to engage
         if mem.enemyclose then
            local dist = ai.dist( enemy )
            if mem.enemyclose > dist then
               attack = true
            end
         else
            attack = true
         end
      end

      -- See what decision to take
      if attack then
         ai.hostile(enemy) -- Should be done before taunting
         taunt(enemy, true)
         ai.pushtask("attack", enemy)
      else
         idle()
      end

   -- Don't stop boarding
   elseif task == "board" then
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
         ai.pushtask("runaway", target)

      -- Think like normal
      else
         attack_think()
      end

      -- Handle distress
      if mem.distress then
         gen_distress()
      end

   -- Pilot is running away
   elseif task == "runaway" then
      target = ai.target()

      -- Needs to have a target
      if not ai.exists(target) then
         ai.poptask()
         return
      end

      local dist = ai.dist( target )

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
      local attack = false

      -- See if enemy is close enough to attack
      if mem.enemyclose then
         local dist = ai.dist( enemy )
         if mem.enemyclose > dist then
            attack = true
         end
      else
         attack = true
      end

      -- See if really want to attack
      if attack then
         taunt(enemy, true)
         ai.pushtask("attack", enemy)
      end
   end
end

-- Required "attacked" function
function attacked ( attacker )
   local task = ai.taskname()
   local target = ai.target()

   -- Notify that pilot has been attacked before
   mem.attacked = true

   if task ~= "attack" and task ~= "runaway" then

      if mem.defensive then
         -- Some taunting
         ai.hostile(attacker) -- Should be done before taunting
         taunt( attacker, false )

         -- Now pilot fights back
         ai.pushtask("attack", attacker)
      else

         -- Runaway
         ai.pushtask("runaway", attacker)
      end

   -- Let attacker profile handle it.
   elseif task == "attack" then
      attack_attacked( attacker )

   elseif task == "runaway" then
      if ai.target() ~= attacker then
         ai.poptask()
         ai.pushtask("runaway", attacker)
      end
   end
end

-- Default task to run when idle
function idle ()
   local planet = ai.landplanet()
   -- planet must exist
   if planet == nil or mem.land_planet == false then
      ai.settimer(0, rnd.int(1000, 3000))
      ai.pushtask("enterdelay")
   else
      mem.land = planet
      ai.pushtask("hyperspace")
      if not mem.tookoff then
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

function create ()
   create_post()
end

-- Finishes create stuff like choose attack and prepare plans
function create_post ()
   mem.tookoff    = ai.takingoff()
   attack_choose()
end

-- taunts
function taunt ( target, offensive )
   -- Empty stub
end


-- Handle distress signals
function distress ( pilot, attacker )

   -- Make sure target exists
   if not ai.exists( attacker ) then
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

   local task = ai.taskname()
   -- If not attacking nor fleeing, begin attacking
   if task ~= "attack" and task ~= "runaway" then
      ai.pushtask( "attack", t )
   -- We're sort of busy
   elseif task == "attack" then
      local target = ai.target()

      if not ai.exists(target) or ai.dist(target) > ai.dist(t) then
         ai.pushtask( "attack", t )
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
      if mem.distressmsgfunc ~= nil then
         mem.distressmsgfunc()
      else
         ai.distress( mem.distressmsg )
      end
      mem.distressed = 1
   end

end

