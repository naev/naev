include("dat/ai/include/basic.lua")
include("dat/ai/include/attack.lua")
local formation = include("dat/scripts/formation.lua")

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
mem.norun = false -- Do not run away.
mem.careful       = false -- Should the pilot try to avoid enemies?

mem.formation     = "circle" -- Formation to use when commanding fleet
mem.form_pos      = nil -- Position in formation (for follower)

--[[Control parameters: mem.radius and mem.angle are the polar coordinates 
of the point the pilot has to follow when using follow_accurate.
The reference direction is the target's velocity direction.
For example, radius = 100 and angle = 180 means that the pilot will stay
behing his target at a distance of 100 units.
angle = 90 will make the pilot try to be on the left of his target,
angle = 0 means that the pilot tries to be in front of the target.]]
mem.radius         = 100 --  Requested distance between follower and target
mem.angle          = 180 --  Requested angle between follower and target's velocity
mem.Kp             = 10 --  First control coefficient
mem.Kd             = 20 -- Second control coefficient


-- Required control rate
control_rate   = 2

function lead_fleet ()
   if #ai.pilot():followers() ~= 0 then
      if mem.formation == nil then
         formation.clear(ai.pilot())
         return
      end

      local form = formation[mem.formation]
      if form == nil then
         warn(string.format("Formation '%s' not found", mem.formation))
      else
         form(ai.pilot())
      end
   end
end

-- Run instead of "control" when under manual control; use should be limited
function control_manual ()
   lead_fleet()
end

function handle_messages ()
   for _, v in ipairs(ai.messages()) do
      local sender, msgtype, data = unpack(v)
      if sender == ai.pilot():leader() then
         if msgtype == "form-pos" then
            mem.form_pos = data
         elseif msgtype == "hyperspace" then
            ai.pushtask("hyperspace", data)
         elseif msgtype == "land" then
            -- TODO: Made sure planet is the same
            mem.land = ai.landplanet():pos()
            ai.pushtask("land")
         -- Escort commands
         -- Attack target
         elseif msgtype == "e_attack" then
            if data ~= nil and data:exists() then
               ai.pushtask("attack", data)
            end
         -- Hold position
         elseif msgtype == "e_hold" then
            ai.pushtask("hold" )
         -- Return to carrier
         elseif msgtype == "e_return" then
            if ai.pilot():flags().carried then
               ai.pushtask("flyback" )
            end
         -- Clear orders
         elseif msgtype == "e_clear" then
            ai.pilot():taskClear()
         end
      end
   end
end

-- Required "control" function
function control ()
   local enemy = ai.getenemy()

   local parmour, pshield = ai.pilot():health()

   lead_fleet()
   handle_messages()

   local task = ai.taskname()

   -- TODO: Select new leader
   if ai.pilot():leader() ~= nil and not ai.pilot():leader():exists() then
      ai.pilot():setLeader(nil)
   end

   -- Cooldown completes silently.
   if mem.cooldown then
      mem.tickssincecooldown = 0

      cooldown, braking = ai.pilot():cooldown()
      if not (cooldown or braking) then
         mem.cooldown = false
      end
   else
      mem.tickssincecooldown = mem.tickssincecooldown + 1
   end

   -- Reset distress if not fighting/running
   if task ~= "attack" and task ~= "runaway" then
      mem.attacked = nil
      local p = ai.pilot()

      -- Cooldown shouldn't preempt boarding, either.
      if task ~= "board" then
         -- Cooldown preempts everything we haven't explicitly checked for.
         if mem.cooldown then
            return
         -- If the ship is hot and shields are high, consider cooling down.
         elseif pshield > 50 and p:temp() > 300 then
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
   if task == nil then
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
      elseif ai.pilot():leader() and ai.pilot():leader():exists() then
         ai.pushtask("follow_fleet")
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
      if not target:exists() then
         ai.poptask()
         return
      end

      local target_parmour, target_pshield = target:health()

      -- Pick an appropriate weapon set.
      choose_weapset()

      -- Runaway if needed
      if (mem.shield_run > 0 and pshield < mem.shield_run
               and pshield < target_pshield ) or
            (mem.armour_run > 0 and parmour < mem.armour_run
               and parmour < target_parmour ) then
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
      if mem.norun or ai.pilot():leader() ~= nil then
         ai.poptask()
         return
      end
      target = ai.target()

      -- Needs to have a target
      if not target:exists() then
         ai.poptask()
         return
      end

      local dist = ai.dist( target )

      -- Should return to combat?
      if mem.aggressive and ((mem.shield_return > 0 and pshield >= mem.shield_return) or
            (mem.armour_return > 0 and parmour >= mem.armour_return)) then
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
      local _, pshield = ai.pilot():health()
      if pshield < 90 then
         mem.cooldown = false
         ai.pilot():setCooldown( false )
      else
         return
      end
   end

   -- Ignore hits from dead pilots.
   if not attacker:exists() then
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
   mem.tookoff    = ai.pilot():flags().takingoff
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

   pfact  = pilot:faction()
   afact  = attacker:faction()
   aifact = ai.pilot():faction()
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
         t = attacker
      end
   elseif mem.aggressive then
      -- Aggressive ships follow their brethren into battle!
      if afact == aifact then
         t = pilot
      elseif p_ally then
         -- When your allies are fighting, stay out of it.
         if a_ally then
            return
         end

         -- Victim is an ally, but the attacker isn't.
         t = attacker
      -- Victim isn't an ally. Attack the victim if the attacker is our ally.
      elseif a_ally then
         t = pilot
      elseif p_enemy then
         -- If they're both enemies, may as well let them destroy each other.
         if a_enemy then
            return
         end

         t = pilot
      elseif a_enemy then
         t = attacker
      -- We'll be nice and go after the aggressor if the victim is peaceful.
      elseif not pilot:memory().aggressive then
         t = attacker
      -- An aggressive, neutral ship is fighting another neutral ship. Who cares?
      else
         return
      end
   -- Non-aggressive ships will flee if their enemies attack neutral or allied vessels.
   elseif a_enemy and not p_enemy then
      t = attacker
   else
      return
   end

   local task = ai.taskname()
   -- We're sort of busy
   if task == "attack" then
      local target = ai.target()

      if not target:exists() or ai.dist(target) > ai.dist(t) then
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
      local p = ai.pilot()
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
   local mean = ai.pilot():weapsetHeat()
   local _, pshield = ai.pilot():health()

   -- Don't want to cool down again so soon.
   -- By default, 15 ticks will be 30 seconds.
   if mem.tickssincecooldown < 15 then
      return
   -- The weapons are extremely hot and cooldown should be triggered.
   -- This did not work before. However now it causes ships to just stop dead and wait for energy regen.
   -- Not sure this is better...
   elseif mean > mem.heatthreshold and pshield > 50 then
      mem.cooldown = true
      ai.pilot():setCooldown(true)
   end
   if pshield == nil then
      player.msg("pshield = nil")
   end
end


-- Holds position
function hold ()
   if not ai.isstopped() then
      ai.brake()
   else
      ai.stop()
   end
end


-- Tries to fly back to carrier
function flyback ()
   local target = ai.pilot():leader()
   local dir    = ai.face(target)
   local dist   = ai.dist(target)
   local bdist  = ai.minbrakedist()

   -- Try to brake
   if not ai.isstopped() and dist < bdist then
      ai.pushtask("brake")

   -- Try to dock
   elseif ai.isstopped() and dist < 30 then
      ai.dock(target)

   -- Far away, must approach
   elseif dir < 10 then
      ai.accel()
   end
end
