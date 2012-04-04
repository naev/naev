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
mem.cooldown       = false -- Whether the pilot is currently cooling down.
mem.safe_distance  = 300 -- Safe distance from enemies to jump
mem.land_planet    = true -- Should land on planets?
mem.land_friendly  = false -- Only land on friendly planets?
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

      -- If the ship is hot, consider cooling down.
      local pilot = ai.getPilot()
      if not mem.cooldown and pilot:temp() > 300 then
         -- Ship is quite hot, better cool down.
         if pilot:temp() > 400 then
            mem.cooldown = true
            ai.getPilot():setCooldown(true)
            return
         end

         local _,wset = pilot:weapset()
         for k,v in ipairs(wset) do
            -- Cool down if a weapon is suffering >= 20% accuracy loss.
            -- This equates to a temperature of 560K presently.
            if v.temp and v.temp > 0.2 then
               mem.cooldown = true
               ai.getPilot():setCooldown(true)
               return
            end
         end
      end
   end

   -- Get new task
   if task == "none" then
      local attack = false

      -- We don't want to interrupt cooldown.
      if mem.cooldown then
         return
      end

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

         -- Cancel cooldown if necessary.
         if mem.cooldown then
            mem.cooldown = false
            ai.getPilot():setCooldown( false )
         end

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

   -- Make sure pilot is setting his target properly
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
   -- If not attacking nor fleeing, begin attacking
   if task ~= "attack" and task ~= "runaway" then
      if mem.aggressive then
         ai.pushtask( "attack", t )
      else
         ai.pushtask( "runaway", t )
      end
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

