include("dat/ai/include/basic.lua")
include("dat/ai/include/attack.lua")

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
mem.cooldown       = false -- Whether the pilot is currently cooling down.
mem.heatthreshold  = .5 -- Weapon heat to enter cooldown at [0-2 or nil]
mem.safe_distance  = 300 -- Safe distance from enemies to jump
mem.land_planet    = true -- Should land on planets?
mem.land_friendly  = false -- Only land on friendly planets?
mem.distress       = true -- AI distresses
mem.distressrate   = 3 -- Number of ticks before calling for help
mem.distressmsg    = nil -- Message when calling for help
mem.distressmsgfunc = nil -- Function to call when distressing
mem.weapset = 3 -- Weapon set that should be used (tweaked based on heat).
mem.tickssincecooldown = 0 -- Prevents overly-frequent cooldown attempts.


-- Required control rate
control_rate   = 2

-- Required "control" function
function control ()
   local task = ai.taskname()
   local enemy = ai.getenemy()

   -- Cooldown completes silently.
   if mem.cooldown then
      mem.tickssincecooldown = 0

      cooldown, braking = ai.getPilot():cooldown()
      if not (cooldown or braking) then
         mem.cooldown = false
      end
   else
      mem.tickssincecooldown = mem.tickssincecooldown + 1
   end

   -- Reset distress if not fighting/running
   if task ~= "attack" and task ~= "runaway" then
      mem.attacked = nil
      local p = ai.getPilot()

      -- Cooldown shouldn't preempt boarding, either.
      if task ~= "board" then
         -- Cooldown preempts everything we haven't explicitly checked for.
         if mem.cooldown then
            return
         -- If the ship is hot and shields are high, consider cooling down.
         elseif ai.pshield() > 50 and p:temp() > 300 then
            -- Ship is quite hot, better cool down.
            if p:temp() > 400 then
               mem.cooldown = true
               p:setCooldown(true)
               return
            -- Cool down if the current weapon set is suffering from >= 20% accuracy loss.
            -- This equates to a temperature of 560K presently.
            elseif (p:weapsetHeat() > .2) then
               mem.cooldown = true
               p:setCooldown(true)
               return
            end
         end
      end
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

      -- Pick an appropriate weapon set.
      choose_weapset()

      -- Runaway if needed
      if (mem.shield_run > 0 and ai.pshield() < mem.shield_run
               and ai.pshield() < ai.pshield(target) ) or
            (mem.armour_run > 0 and ai.parmour() < mem.armour_run
               and ai.parmour() < ai.parmour(target) ) then
         ai.pushtask("runaway", target)

      -- Think like normal
      else
         -- Cool down, if necessary.
         should_cooldown()

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
      -- Don't start new attacks while refueling.
      if task == "refuel" then
         return
      end

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

   -- Cooldown should be left running if not taking heavy damage.
   if mem.cooldown then
      if ai.pshield() < 90 then
         mem.cooldown = false
         ai.getPilot():setCooldown( false )
      else
         return
      end
   end

   -- Ignore hits from dead pilots.
   if not ai.exists(attacker) then
      return
   end

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
   if not attacker:exists() then
      return
   end

   -- Make sure pilot is setting their target properly
   if pilot == attacker then
      return
   end

   pid    = pilot:id()
   aid    = attacker:id()
   pfact  = pilot:faction()
   afact  = attacker:faction()
   aifact = ai.getPilot():faction()
   p_ally  = aifact:areAllies(pfact)
   a_ally  = aifact:areAllies(afact)
   p_enemy = aifact:areEnemies(pfact)
   a_enemy = aifact:areEnemies(afact)

   -- Ships should always defend their brethren.
   if pfact == aifact then
      -- We don't want to cause a complete breakdown in social order.
      if afact == aifact then
         return
      else
         t = aid
      end
   elseif mem.aggressive then
      -- Aggressive ships follow their brethren into battle!
      if afact == aifact then
         t = pid
      elseif p_ally then
         -- When your allies are fighting, stay out of it.
         if a_ally then
            return
         end

         -- Victim is an ally, but the attacker isn't.
         t = aid
      -- Victim isn't an ally. Attack the victim if the attacker is our ally.
      elseif a_ally then
         t = pid
      elseif p_enemy then
         -- If they're both enemies, may as well let them destroy each other.
         if a_enemy then
            return
         end

         t = pid
      elseif a_enemy then
         t = aid
      -- We'll be nice and go after the aggressor if the victim is peaceful.
      elseif not pilot:memoryCheck("aggressive") then
         t = aid
      -- An aggressive, neutral ship is fighting another neutral ship. Who cares?
      else
         return
      end
   -- Non-aggressive ships will flee if their enemies attack neutral or allied vessels.
   elseif a_enemy and not p_enemy then
      t = aid
   else
      return
   end

   local task = ai.taskname()
   -- We're sort of busy
   if task == "attack" then
      local target = ai.target()

      if not ai.exists(target) or ai.dist(target) > ai.dist(t) then
         ai.pushtask( "attack", t )
      end
   -- If not fleeing or refueling, begin attacking
   elseif task ~= "runaway" and task ~= "refuel" then
      if mem.aggressive then
         ai.pushtask( "attack", t )
      else
         ai.pushtask( "runaway", t )
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

   -- Initialize if unset.
   if mem.distressed == nil then
      mem.distressed = 1
   end

   -- Update distress counter
   mem.distressed = mem.distressed + 1

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


-- Picks an appropriate weapon set for ships with mixed weaponry.
function choose_weapset()
   if ai.hascannons() and ai.hasturrets() then
      local p = ai.getPilot()
      local meant, peakt = p:weapsetHeat( 3 )
      local meanc, peakc = p:weapsetHeat( 2 )

      --[[
      -- Weapon groups:
      --    1: Cannons
      --    2: Turrets
      --    3: Combined
      --
      -- Note: AI indexes from 0, but pilot module indexes from 1.
      --]]

      -- Use both if both are cool, or if both are similar in temperature.
      if meant + meanc < .1 then
         mem.weapset = 3
      elseif peakt == 0 then
         mem.weapset = 2
      elseif peakc == 0 then
         mem.weapset = 1
      -- Both sets are similarly hot.
      elseif math.abs(meant - meanc) < .15 then
         mem.weapset = 3
      -- An extremely-hot weapon is a good reason to pick another set.
      elseif math.abs(peakt - peakc) > .4 then
         if peakt > peakc then
            mem.weapset = 1
         else
            mem.weapset = 2
         end
      elseif meant > meanc then
         mem.weapset = 1
      else
         mem.weapset = 2
      end
   end
end

-- Puts the pilot into cooldown mode if its weapons are overly hot and its shields are relatively high.
-- This can happen during combat, so mem.heatthreshold should be quite high.
function should_cooldown()
   local mean = ai.getPilot():weapsetHeat()

   -- Don't want to cool down again so soon.
   -- By default, 15 ticks will be 30 seconds.
   if mem.tickssincecooldown < 15 then
      return
   -- The weapons are extremely hot and cooldown should be triggered.
   elseif mean > mem.heatthreshold and ai.pshield() > 50 then
      mem.cooldown = true
      ai.getPilot():setCooldown(true)
   end
end
