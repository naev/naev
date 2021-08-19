local formation = require("scripts/formation")

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
mem.heatthreshold  = 0.5 -- Weapon heat to enter cooldown at [0-2 or nil]
mem.safe_distance  = 300 -- Safe distance from enemies to jump
mem.land_planet    = true -- Should land on planets?
mem.land_friendly  = false -- Only land on friendly planets?
mem.distress       = true -- AI distresses
mem.distressrate   = 3 -- Number of ticks before calling for help
mem.distressmsg    = nil -- Message when calling for help
mem.distressmsgfunc = nil -- Function to call when distressing
mem.weapset        = 3 -- Weapon set that should be used (tweaked based on heat).
mem.tickssincecooldown = 0 -- Prevents overly-frequent cooldown attempts.
mem.norun         = false -- Do not run away.
mem.careful       = false -- Should the pilot try to avoid enemies?
mem.doscans       = false -- Should the pilot try to scan other pilots?
-- mem.scanned is created per pilot otherwise the list gets "shared"
--mem.scanned       = {} -- List of pilots that have been scanned

mem.formation     = "circle" -- Formation to use when commanding fleet
mem.form_pos      = nil -- Position in formation (for follower)
mem.leadermaxdist = nil -- Distance from leader to run back to leader
mem.gather_range  = 800 -- Radius in which the pilot looks for gatherables

--[[Control parameters: mem.radius and mem.angle are the polar coordinates
of the point the pilot has to follow when using follow_accurate.
The reference direction is the target's velocity direction.
For example, radius = 100 and angle = 180 means that the pilot will stay
behind his target at a distance of 100 units.
angle = 90 will make the pilot try to be on the left of his target,
angle = 0 means that the pilot tries to be in front of the target.]]
mem.radius         = 100 --  Requested distance between follower and target
mem.angle          = 180 --  Requested angle between follower and target's velocity
mem.Kp             = 10 --  First control coefficient
mem.Kd             = 20 -- Second control coefficient

mem.target_bias      = vec2.new(0,0) -- Initialize land bias, just in case

-- Required control rate that represents the number of seconds between each
-- control() call
control_rate   = 2

--[[
   Binary flags for the different states that default to nil (false).
   - attack: the pilot is attacked their target
   - fighting: the pilot is engaged in combat (including running away )
   - noattack: do not try to find new targets to attack
--]]
stateinfo = {
   attack = {
      fighting = true,
      attack = true,
   },
   attack_forced = {
      forced = true,
      fighting = true,
      attack = true,
      noattack = true,
   },
   runaway = {
      fighting = true,
      noattack = true,
   },
   refuel = {
      noattack = true,
   },
   hold = {
      forced = true,
      noattack = true,
   },
   flyback = {
      forced = true,
      noattack = true,
   },
}
function _stateinfo( task )
   if task == nil then
      return {}
   end
   return stateinfo[ task ] or {}
end

function lead_fleet ()
   local p = ai.pilot()
   if #p:followers() ~= 0 then
      if mem.formation == nil then
         formation.clear(p)
         return
      end

      local form = formation[mem.formation]
      if form == nil then
         warn(string.format(_("Pilot '%s': formation '%s' not found!"), p:name(), mem.formation))
      else
         form(p)
      end
   end
end

-- Run instead of "control" when under manual control; use should be limited
function control_manual ()
   lead_fleet()
end

function handle_messages( si )
   local p = ai.pilot()
   local l = p:leader()
   for _, v in ipairs(ai.messages()) do
      local sender, msgtype, data = table.unpack(v)

      if not sender:exists() then
         return
      end

      -- Below we only handle if they came from allies
      -- (So far, only allies would send in the first place, but this check future-proofs things.
      -- One day it might be interesting to have non-allied snitches whose tips get checked out...)
      if p:faction():areAllies( sender:faction() ) then
         if msgtype == "scanned" then
            if mem.doscans and data ~= nil and data:exists() then
               mem.scanned = mem.scanned or {} -- Create table if doesn't exist
               table.insert( mem.scanned, data )
               -- Stop scanning if they got information about the scan
               if ai.taskname() == "scan" and ai.taskdata() == data then
                  ai.poptask()
               end
            end
         end
      end

      -- Messages coming from followers
      if sender:leader() == p then
         if msgtype == "f_attacked" then
            if not si.fighting and should_attack( data, si ) then
               ai.pushtask("attack", data)
               -- Also signal to other followers
               for k,v in ipairs(p:followers()) do
                  p:msg( v, "l_attacked", data )
               end
            end
         end

      -- Below we only handle if they came from the glorious leader
      elseif sender == l then
         if msgtype == "form-pos" then
            mem.form_pos = data
         elseif msgtype == "hyperspace" then
            ai.pushtask("hyperspace", data)
         elseif msgtype == "land" then
            ai.pushtask("land", data)
         elseif msgtype == "l_attacked" then
            if not si.fighting and should_attack( data, si ) then
               ai.pushtask("attack", data)
            end

         -- Escort commands
         -- Attack target
         elseif msgtype == "e_attack" then
            if data ~= nil and data:exists() then
               if data:leader() ~= l then
                  clean_task( ai.taskname() )
                  ai.pushtask("attack_forced", data)
               end
            end
         -- Hold position
         elseif msgtype == "e_hold" then
            ai.pushtask("hold")
         -- Return to carrier
         elseif msgtype == "e_return" then
            ai.pushtask( "flyback", p:flags("carried") )
         -- Clear orders
         elseif msgtype == "e_clear" then
            p:taskClear()
         end
      end
   end
end

function should_attack( enemy, si )
   if not enemy or not enemy:exists() then
      return false
   end

   if not mem.aggressive then
      return false
   end

   -- Don't reattack the current enemy
   if si.attack and enemy == ai.taskdata() then
      return false
   end

   -- Check if we have minimum range to engage
   if mem.enemyclose then
      local dist = ai.dist2( enemy )
      if mem.enemyclose*mem.enemyclose > dist then
         return true
      end
   else
      return true
   end
   return false
end

function control_attack( si )
   local target = ai.taskdata()
   -- Needs to have a target
   if not target or not target:exists() then
      ai.poptask()
      return
   end

   local target_parmour, target_pshield = target:health()
   local parmour, pshield = ai.pilot():health()

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

      attack_think( target, si )
   end

   -- Handle distress
   if mem.distress then
      gen_distress()
   end
end

-- Required "control" function
function control ()
   local p = ai.pilot()
   local enemy = ai.getenemy()

   -- Task information stuff
   local task = ai.taskname()
   local si = _stateinfo( task )

   lead_fleet()
   handle_messages( si )

   -- Select new leader
   local l = p:leader()
   if l ~= nil and not l:exists() then
      local candidate = ai.getBoss()
      if candidate ~= nil and candidate:exists() then
         p:setLeader( candidate )
         l = candidate
      else -- Indicate this pilot has no leader
         p:setLeader( nil )
         l = nil
      end
   end

   -- Try to stealth if leader is stealthed
   if l ~= nil then
      if l:flags("stealth") then
         ai.stealth(true)
      elseif p:flags("stealth") then
         ai.stealth(false)
      end
   end

   -- If command is forced we basically override everything
   if si.forced then
      if si.attack then
         control_attack( si )
      end
      return
   end

   -- Cooldown completes silently.
   if mem.cooldown then
      mem.tickssincecooldown = 0

      local cooldown, braking = p:cooldown()
      if not (cooldown or braking) then
         mem.cooldown = false
      end
   else
      mem.tickssincecooldown = mem.tickssincecooldown + 1
   end

   -- Reset distress if not fighting/running
   if not si.fighting then
      mem.attacked = nil

      -- Cooldown shouldn't preempt boarding, either.
      if task ~= "board" then
         -- Cooldown preempts everything we haven't explicitly checked for.
         if mem.cooldown then
            return
         -- If the ship is hot, consider cooling down.
         elseif p:temp() > 300 then
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

   -- Pilots return if too far away from leader
   local lmd = mem.leadermaxdist
   if lmd then
      local l = p:leader()
      if l then
         local dist = ai.dist( l )
         if lmd < dist then
            if task ~= "flyback" then
               ai.pushtask("flyback", false)
            end
            return
         end
      end
   end

   -- Get new task
   if task == nil then
      -- See what decision to take
      if should_attack( enemy, si ) then
         ai.hostile(enemy) -- Should be done before taunting
         taunt(enemy, true)
         ai.pushtask("attack", enemy)
      elseif p:leader() and p:leader():exists() then
         ai.pushtask("follow_fleet")
      else
         idle()
      end

   end


   -- Run custom function if applicable
   if task then
      local cc = control_funcs[ task ]
      if cc then
         if cc() then
            return
         end
      end
   end

   -- Enemy sighted, handled doing specific tasks
   if enemy ~= nil and mem.aggressive then
      -- Don't start new attacks while refueling.
      if si.noattack then
         return
      end

      -- See if really want to attack
      if should_attack( enemy, si ) then
         ai.hostile(enemy) -- Should be done before taunting
         taunt(enemy, true)
         clean_task( task )
         ai.pushtask("attack", enemy)
      end
   end
end

control_funcs = {}
function control_funcs.loiter ()
   if mem.doscans and rnd.rnd() < 0.1 then
      local target = __getscantarget()
      if target then
         __push_scan( target )
      end
   end
   return false
end
function control_funcs.runaway ()
   if mem.norun or ai.pilot():leader() ~= nil then
      ai.poptask()
      return true
   end
   local target = ai.taskdata()

   -- Needs to have a target
   if not target:exists() then
      ai.poptask()
      return true
   end

   local dist = ai.dist( target )

   -- Should return to combat?
   local parmour, pshield = ai.pilot():health()
   if mem.aggressive and ((mem.shield_return > 0 and pshield >= mem.shield_return) or
         (mem.armour_return > 0 and parmour >= mem.armour_return)) then
      ai.poptask() -- "attack" should be above "runaway"

   -- Try to jump
   elseif dist > mem.safe_distance then
      ai.hyperspace()
   end

   -- Handle distress
   gen_distress()
   return true
end
function control_funcs.board ()
   local task = ai.taskname()
   local si = _stateinfo( task )

   -- Needs to have a target
   local target = ai.taskdata()
   if not target or not target:exists() then
      ai.poptask()
      return true
   end
   -- We want to think in case another attacker gets close
   attack_think( target, si )
   return true
end
function control_funcs.attack ()
   local task = ai.taskname()
   local si = _stateinfo( task )
   control_attack( si )
   return false
end
function control_funcs.attack_forced ()
   -- Independent of control_funcs.attack
   local task = ai.taskname()
   local si = _stateinfo( task )
   control_attack( si )
   return true
end
function control_funcs.flyback () return true end
function control_funcs.hold () return true end

-- Required "attacked" function
function attacked( attacker )
   local task = ai.taskname()
   local si = _stateinfo( task )
   if si.forced then return end

   -- Notify that pilot has been attacked before
   mem.attacked = true
   local p = ai.pilot()

   -- Pilot shouldn't be allowed to rebribe, so we just have to cancel
   -- bribe status
   if ai.isbribed(attacker) then
      p:setBribed( false )
   end

   -- Cooldown should be left running if not taking heavy damage.
   if mem.cooldown then
      local _, pshield = p:health()
      if pshield < 90 then
         mem.cooldown = false
         p:setCooldown( false )
      else
         return
      end
   end

   -- Ignore hits from dead pilots.
   if not attacker:exists() then
      return
   end

   -- Notify followers that we've been attacked
   if not si.fighting then
      for k,v in ipairs(p:followers()) do
         p:msg( v, "l_attacked", attacker )
      end
      local l = p:leader()
      if l and l:exists() then
         p:msg( l, "f_attacked", attacker )
      end
   end

   if not si.fighting then

      if mem.defensive then
         -- Some taunting
         ai.hostile(attacker) -- Should be done before taunting
         taunt( attacker, false )

         -- Now pilot fights back
         clean_task( task )
         ai.pushtask("attack", attacker)
      else

         -- Runaway
         ai.pushtask("runaway", attacker)
      end

   -- Let attacker profile handle it.
   elseif si.attack then
      attack_attacked( attacker )

   elseif task == "runaway" then
      if ai.taskdata() ~= attacker then
         ai.poptask()
         ai.pushtask("runaway", attacker)
      end
   end
end

function create ()
   create_post()
end

-- Finishes create stuff like choose attack and prepare plans
function create_post ()
   local p = ai.pilot()
   mem.tookoff    = p:flags("takingoff")
   mem.jumpedin   = p:flags("jumpingin")
   mem.scanned    = {} -- must create for each pilot
   attack_choose()

   -- Give a small delay
   if mem.jumpedin then
      ai.settimer( 0, rnd.uniform(3.0, 5.0) )
      ai.pushtask("idle_wait")
   end
end

-- taunts
function taunt ( target, offensive )
   -- Empty stub
end


-- Handle distress signals
function distress ( pilot, attacker )
   local p = ai.pilot()

   -- Make sure target exists
   if not attacker:exists() then return end

   -- Make sure pilot is setting their target properly
   if pilot == attacker then return end

   -- Ignore pleas of help when bribed by the attacker
   if ai.isbribed(attacker) then return end

   local pfact   = pilot:faction()
   local afact   = attacker:faction()
   local aifact  = p:faction()
   local p_ally  = aifact:areAllies(pfact)
   local a_ally  = aifact:areAllies(afact)
   local p_enemy = aifact:areEnemies(pfact)
   local a_enemy = aifact:areEnemies(afact)

   local badguy
   -- Victim is ally
   if p_ally then
      -- When your allies are fighting, stay out of it.
      if a_ally then
         return
      end
      -- Victim is an ally, but the attacker isn't.
      badguy = attacker
   -- Victim isn't an ally. Attack the victim if the attacker is our ally.
   elseif a_ally then
      badguy = pilot
   elseif p_enemy then
      -- If they're both enemies, may as well let them destroy each other.
      if a_enemy then
         return
      end
      badguy = pilot
   elseif a_enemy then
      badguy = attacker
   -- We'll be nice and go after the aggressor if the victim is peaceful.
   elseif not pilot:memory().aggressive then
      badguy = attacker
   end

   -- Cannot discern the bad guy, so just look the other way
   if not badguy then return end

   local task = ai.taskname()
   local si   = _stateinfo( task )
   -- Already fighting
   if si.attack then
      if si.noattack then return end
      -- Ignore if not interested in attacking
      if not should_attack( badguy, si ) then return end

      local target = ai.taskdata()

      -- See if we want to switch targets
      if not target:exists() or ai.dist2(target) > ai.dist2(badguy) then
         if p:inrange( badguy ) then
            ai.pushtask( "attack", badguy )
         end
      end
   -- If not fleeing or refueling, begin attacking
   elseif task ~= "runaway" and task ~= "refuel" then
      if not si.noattack and mem.aggressive then
         -- Ignore if not interested in attacking
         if not should_attack( badguy, si ) then return end
         if p:inrange( badguy ) then -- TODO: something to help in the other case
            clean_task( task )
            ai.pushtask( "attack", badguy )
         end
      else
         ai.pushtask( "runaway", badguy )
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
      if mem.distressmsgfunc then
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


-- Decide if the task is likely to become obsolete once attack is finished
function clean_task( task )
   if task == "brake" or task == "inspect_moveto" then
      ai.poptask()
   end
end
