local atklib = require "ai.core.attack"
local atk = require "ai.core.attack.util"
local atk_generic = require "ai.core.attack.generic"
local fmt = require "format"
local formation = require "formation"
local lanes = require 'ai.core.misc.lanes'
local scans = require 'ai.core.misc.scans'

local choose_weapset, clean_task, gen_distress, gen_distress_attacked, handle_messages, lead_fleet, should_cooldown, consider_taunt -- Forward-declared functions

--[[
-- Variables to adjust AI
--
-- These variables can be used to adjust the generic AI to suit other roles.
--]]
mem.atk_changetarget = 2 -- Distance at which target changes
mem.atk_approach  = 1.4 -- Distance that marks approach
mem.atk_aim       = 1.0 -- Distance that marks aim
mem.atk_board     = false -- Whether or not to board the target
mem.atk_kill      = true -- Whether or not to finish off the target
mem.atk_minammo   = 0.1 -- Percent of ammo necessary to do ranged attacks
mem.vulnattack    = 1.5 -- Vulnerability threshold
mem.ranged_ammo   = 0 -- How much ammo is left, we initialize to 0 here just in case
mem.recharge      = false --whether to hold off shooting to avoid running dry of energy
mem.enemyclose    = nil -- Distance at which an enemy is considered close
mem.armour_run    = -1 -- At which damage to run at
mem.armour_return = 0 -- At which armour to return to combat
mem.shield_run    = -1 -- At which shield to run
mem.shield_return = 0 -- At which shield to return to combat
mem.aggressive    = false -- Should pilot actively attack enemies?
mem.defensive     = true -- Should pilot defend itself
mem.whiteknight   = false -- Should the AI help out independent ships duking it out?
mem.cooldown      = false -- Whether the pilot is currently cooling down.
mem.heatthreshold = 0.5 -- Weapon heat to enter cooldown at [0-2 or nil]
mem.safe_distance = 8000 -- Safe distance from enemies to stop running away
mem.safe_jump_distance = 300 -- Safe distance from enemies to jump
mem.land_planet   = true -- Should land on planets?
mem.land_friendly = false -- Only land on friendly planets?
mem.distress      = true -- AI distresses
mem.distress_hit  = 0 -- Amount of faction lost on distress
mem.distressrate  = 3 -- Number of ticks before calling for help
mem.distressmsg   = nil -- Message when calling for help
mem.distressmsgfunc = nil -- Function to call when distressing
mem.weapset       = 3 -- Weapon set that should be used (tweaked based on heat).
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
mem.lanes_useneutral = false -- Whether or not to use neutral lanes

--[[Control parameters: mem.radius and mem.angle are the polar coordinates
of the point the pilot has to follow when using follow_accurate.
The reference direction is the target's velocity direction.
For example, radius = 100 and angle = math.pi means that the pilot will stay
behind his target at a distance of 100 units.
angle = math.pi/2 will make the pilot try to be on the left of his target,
angle = 0 means that the pilot tries to be in front of the target.]]
mem.radius         = 100 --  Requested distance between follower and target
mem.angle          = math.pi --  Requested angle between follower and target's velocity
mem.Kp             = 10 --  First control coefficient
mem.Kd             = 20 -- Second control coefficient (this value is overwritten in create_post)

mem.target_bias    = vec2.new(0,0) -- Initialize land bias, just in case

mem.elapsed       = 0 -- Time elapsed since pilot was created

-- Required control rate that represents the number of seconds between each
-- control() call
control_rate   = 2

--[[
   Binary flags for the different states that default to nil (false).
   - forced: the task is forced and shouldn't be changed
   - attack: the pilot is attacking their target
   - fighting: the pilot is engaged in combat (including running away )
   - noattack: do not try to find new targets to attack
--]]
local stateinfo = {
   attack = {
      fighting = true,
      attack   = true,
   },
   attack_forced = {
      forced   = true,
      fighting = true,
      attack   = true,
      noattack = true,
   },
   attack_forced_kill = {
      forced   = true,
      fighting = true,
      attack   = true,
      noattack = true,
   },
   return_lane = {
      running  = true,
      noattack = true,
   },
   backoff = {
      running  = true,
      fighting = true,
      noattack = true,
   },
   runaway = {
      running  = true,
      fighting = true,
      noattack = true,
   },
   refuel = {
      noattack = true,
   },
   hold = {
      forced   = true,
      noattack = true,
   },
   flyback = {
      forced   = true,
      noattack = true,
   },
   jumpin_wait = {
      noattack = true,
   },
}
local function _stateinfo( task )
   if task == nil then
      return {}
   end
   return stateinfo[ task ] or {}
end

--[[
   Attack wrappers for calling the correct attack functions.
--]]

--[[
-- Wrapper for the attack functions.
--]]
function attack( target )
   -- Don't go on the offensive when in the middle of cooling.
   if mem.cooldown then
      ai.poptask()
      return
   end

   local lib = (mem.atk or atk_generic)
   lib.atk( target )
end

--[[
-- Forced attack function that should focus on the enemy until done
--]]
function attack_forced( target )
   local lib = (mem.atk or atk_generic)
   lib.atk( target )
end

--[[
-- Forced attack function that should focus on the enemy until enemy is killed
--]]
function attack_forced_kill( target )
   local lib = (mem.atk or atk_generic)
   lib.atk( target, true )
end

function lead_fleet( p )
   if #p:followers() == 0 then
      return
   end
   if mem.formation == nil then
      formation.clear(p)
      return
   end

   local form = formation[mem.formation]
   if form == nil then
      warn(fmt.f(_("Pilot '{plt}': formation '{formation}' not found!"), {plt=p, formation=mem.formation}))
   else
      form(p)
   end
end

-- Run instead of "control" when under manual control; use should be limited
function control_manual( dt )
   mem.elapsed = mem.elapsed + dt
   local p = ai.pilot()
   local task = ai.taskname()
   local si = _stateinfo( task )
   ai.combat( si.fighting )

   lead_fleet( p )
   handle_messages( si, false )
end

function handle_messages( si, dopush )
   local taskchange = false
   local p = ai.pilot()
   local l = p:leader()
   for i, msg in ipairs(ai.messages()) do
      local sender, msgtype, data = msg[1], msg[2], msg[3]

      -- This is the case that the message is being sent from the environment, such as asteroids
      if sender==nil then
         -- Asteroid was blown up with mining tools
         if msgtype=="asteroid" and data and data:exists() then
            local ap = data:pos()
            if dopush and should_investigate( ap, si ) then
               ap = ap + vec2.newP( 500*rnd.rnd(), rnd.angle () )
               ai.pushtask("inspect_moveto", ap )
               taskchange = true
            end

         -- Some signal was detected
         elseif msgtype=="signal" and data then
            if dopush and should_investigate( data, si ) then
               ai.pushtask("inspect_moveto", data )
               taskchange = true
            end
         end
      else

         -- Special case leader is gone but we want to follow, so we ignore if they're non-existent.
         if l==nil or sender==l then
            if msgtype == "hyperspace" then
               if dopush then
                  ai.pushtask("hyperspace", data)
                  taskchange = true
               end
            elseif msgtype == "land" then
               if dopush then
                  ai.pushtask("land", data)
                  taskchange = true
               end
            end
         end

         -- Skip message from nonexistent sender
         if sender:exists() then

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
                        taskchange = true
                     end
                  end
               end
            end

            -- Messages coming from followers
            if sender:leader() == p then
               if msgtype == "f_attacked" then
                  if not si.fighting and should_attack( data, si, true ) then
                     -- Also signal to other followers
                     for k,v in ipairs(p:followers()) do
                        p:msg( v, "l_attacked", data )
                     end
                     if dopush then
                        ai.pushtask("attack", data)
                        taskchange = true
                     end
                  end
               end

            -- Below we only handle if they came from the glorious leader
            elseif sender == l then
               if msgtype == "form-pos" then
                  mem.form_pos = data
               elseif msgtype == "l_attacked" then
                  if not si.fighting and should_attack( data, si, true ) then
                     if dopush then
                        ai.pushtask("attack", data)
                        taskchange = true
                     end
                  end

               -- Escort commands
               elseif dopush then
                  -- Attack target
                  if msgtype == "e_attack" then
                     if data ~= nil and data:exists() then
                        if data:leader() ~= l then -- Don't kill from same team
                           clean_task()
                           --if (si.attack and si.forced and ai.taskdata()==data) or data:flags("disabled") then
                           if data:flags("disabled") then
                              ai.pushtask("attack_forced_kill", data)
                           else
                              ai.pushtask("attack_forced", data)
                           end
                           taskchange = true
                        end
                     end
                  -- Hold position
                  elseif msgtype == "e_hold" then
                     ai.pushtask("hold")
                     taskchange = true
                  -- Return to carrier
                  elseif msgtype == "e_return" then
                     ai.pushtask( "flyback", mem.carried )
                     taskchange = true
                  -- Clear orders
                  elseif msgtype == "e_clear" then
                     p:taskClear()
                     taskchange = true
                  end
               end
            end
         end
      end
   end
   return taskchange
end

--[[
-- Helper function to see if two pilots belong to the same fleet or not
--]]
local function sameFleet( pa, pb )
   local la = pa:leader()
   local lb = pa:leader()
   if not la or not la:exists() then la = pa end
   if not lb or not lb:exists() then lb = pb end
   return la == lb
end

--[[
-- Whether or not the pilot should try to attack an enemy.
--]]
function should_attack( enemy, si, aggressor )
   if not enemy or not enemy:exists() then
      return false
   end

   if not aggressor and not mem.aggressive then
      return false
   end

   si = si or _stateinfo( ai.taskname() )

   if si.noattack then
      return false
   end

   -- Don't reattack the current enemy
   if si.attack and enemy==ai.taskdata() then
      return false
   end

   -- Try to follow the leader behaviour
   local p = ai.pilot()
   local l = p:leader()
   if l then
      local ltask, ldata = l:task()
      local lsi = _stateinfo( ltask )
      if lsi.fighting then
         if ldata and ldata:exists() then
            -- Check to see if the pilot group the leader is fighting is the
            -- same as the current enenmy
            if sameFleet( ldata, enemy ) then
               return true
            end
         end
         -- TODO maybe add a check to see if nearby fighting leader?
      end
      local lmd = mem.leadermaxdist
      if lmd then
         local d = l:pos():dist2( enemy:pos() )
         if d > lmd*lmd then
            return false
         end
      end
   end

   -- Check to see if we want to go back to the lanes
   local lr = mem.enemyclose
   if mem.natural and lr then
      local d, _pos = lanes.getDistance2P( p, enemy:pos() )
      if math.huge > d and d > lr*lr then
         return false
      end
   end

   -- Check if we have minimum range to engage
   if lr then
      local d = ai.dist2( enemy )
      if lr*lr > d then
         return true
      end
   else
      return true
   end
   return false
end

--[[
-- Whether or not the pilot should investigate a certain location.
--]]
function should_investigate( pos, si )
   if si.fighting or si.forced or si.noattack then
      return false
   end

   -- Only care about scanning
   if not mem.doscans or rnd.rnd() < 0.8 then
      return false
   end

   -- Conmfort
   local ec = mem.enemyclose or math.huge
   local ec2 = ec*ec

   -- Should be nearby
   if ai.dist2(pos) > ec2 then
      return false
   end

   -- Check to see if we want to go back to the lanes
   if mem.natural and ec < math.huge then
      local d, _pos = lanes.getDistance2P( ai.pilot(), pos )
      if math.huge > d and d > ec2 then
         return false
      end
   end

   return true
end

--[[
-- Table of think functions for the different tasks (uses task name as an index).
-- These are used after the general thought process, including things like
-- handling messages and running away.
--
-- They should return true if the pilot is not interested in attacking while
-- doing the task, or false otherwise.
--]]
control_funcs = {}
function control_funcs.generic_attack( si, noretarget )
   si = si or _stateinfo( ai.taskname() )
   local target = ai.taskdata()
   -- Needs to have a target
   if not target or not target:exists() then
      ai.poptask()
      return false
   end

   local target_parmour, target_pshield = target:health()
   local parmour, pshield = ai.pilot():health()

   -- Pick an appropriate weapon set.
   choose_weapset()

   -- Runaway if needed
   if not mem.norun and (pshield < mem.shield_run
            and pshield < target_pshield ) or
         (parmour < mem.armour_run
            and parmour < target_parmour ) then
      ai.pushtask("runaway", target)

   -- Think like normal
   else
      -- Cool down, if necessary.
      should_cooldown()

      atklib.think( target, si, noretarget )
   end

   -- Handle distress
   gen_distress( target )
   return false
end

-- Required "control" function
function control( dt )
   mem.elapsed = mem.elapsed + dt
   local p = ai.pilot()

   -- Task information stuff
   local task = ai.taskname()
   local si = _stateinfo( task )
   ai.combat( si.fighting )

   lead_fleet( p )
   local taskchange = handle_messages( si, true )

   -- Select new leader
   local l = p:leader()
   if not mem.carried then -- carried ships don't change
      if l == nil and mem.autoleader then
         local candidate = ai.getBoss()
         if candidate ~= nil and candidate:exists() then
            p:setLeader( candidate )
            l = candidate
         else -- Indicate this pilot has no leader
            p:setLeader( nil )
            l = nil
         end
      end
   end

   -- Try to stealth if leader is stealthed
   if l then
      if l:flags("stealth") then
         ai.stealth(true)
      elseif p:flags("stealth") then
         ai.stealth(false)
      end
   end

   -- Task changed so we stop thinking and focus on new task
   if taskchange then return end

   -- If command is forced we basically override everything
   if si.forced then
      if si.attack then
         control_funcs.generic_attack( si )
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
            elseif (p:weapsetHeat() > 0.2) then
               mem.cooldown = true
               p:setCooldown(true)
               return
            end
         end
      end

      -- Recall fighters if applicable
      if ai.hasfighterbays() then
         for k,v in ipairs(p:followers()) do
            if v:flags("carried") then
               p:msg( v, "e_return" )
            end
         end
      end
   end

   -- Pilots return if too far away from leader
   local lmd = mem.leadermaxdist
   if lmd then
      --local l = p:leader() -- Leader should be set already
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

   -- Check to see if we want to go back to the lanes
   if mem.natural and si.fighting and not si.running then
      local lr = mem.enemyclose
      if lr then
         local d, pos = lanes.getDistance2P( p, p:pos() )
         if d < math.huge and d > lr*lr then
            local enemy = atk.preferred_enemy( nil, true )
            ai.pushtask( "return_lane", {enemy, pos} )
            return
         end
      end
   end

   -- Get new task
   if task == nil then
      local enemy = atk.preferred_enemy( nil, true )
      -- See what decision to take
      if should_attack( enemy, si, false ) then
         ai.hostile(enemy) -- Should be done before taunting
         consider_taunt(enemy, true)
         ai.pushtask("attack", enemy)
      elseif l then -- Leader should be set already
         ai.pushtask("follow_fleet")
      else
         idle()
      end
      return -- Should have gotten a new task
   end

   -- Run custom function if applicable
   task = ai.taskname() -- Reget the task in case something got pushed ontop
   if task then
      local cc = control_funcs[ task ]
      if cc then
         if cc() then
            return
         end
      end
   end

   -- Enemy sighted, handled doing specific tasks
   if mem.aggressive then
      local enemy = atk.preferred_enemy( nil, true )
      -- See if really want to attack
      if should_attack( enemy, si, false ) then
         ai.hostile(enemy) -- Should be done before taunting
         consider_taunt(enemy, true)
         clean_task()
         ai.pushtask("attack", enemy)
      end
   end
end

function control_funcs.loiter ()
   if mem.doscans and rnd.rnd() < 0.1 then
      local target = scans.get_target()
      if target then
         scans.push( target )
      end
   end
   return false
end
control_funcs.loiter_last = control_funcs.loiter
control_funcs.inspect_moveto = function ()
   local p = ai.pilot()
   local target = ai.taskdata()
   local lr = mem.enemyclose
   if mem.natural and target and lr and lanes.getDistance2P( p, target ) > lr*lr then
      ai.poptask()
      return false
   end
   return true
end
function control_funcs.runaway ()
   local p = ai.pilot()
   if mem.norun or p:leader() ~= nil then
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
   local parmour, pshield = p:health()
   if mem.aggressive and ((mem.shield_return > 0 and pshield >= mem.shield_return) or
         (mem.armour_return > 0 and parmour >= mem.armour_return)) then
      ai.poptask() -- "attack" should be above "runaway"
      return true

   -- Try to jump
   elseif dist > mem.safe_jump_distance then
      ai.hyperspace()
   else
      -- If far enough away, stop the task
      local enemy = atk.preferred_enemy() -- nearest enemy
      if not enemy or (enemy and ai.dist(enemy) > mem.safe_distance) then
         ai.poptask()
         return true
      end
   end

   -- Handle distress
   gen_distress( target )
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
   atklib.think( target, si )
   return true
end
function control_funcs.attack ()
   return control_funcs.generic_attack()
end
function control_funcs.attack_forced ()
   -- Independent of control_funcs.attack
   control_funcs.generic_attack( nil, true )
   return true
end
function control_funcs.flyback () return true end
function control_funcs.hold () return true end

function attacked_manual( attacker )
   -- Ignore hits from dead pilots.
   if not attacker:exists() then
      return
   end

   local task = ai.taskname()
   local si = _stateinfo( task )

   -- Notify that pilot has been attacked before
   local p = ai.pilot()
   if not mem.attacked then
      mem.attacked = true
      if ai.hasfighterbays() then
         for k,v in ipairs(p:followers()) do
            p:msg( v, "e_clear" )
         end
      end
   end

   -- Pilot shouldn't be allowed to rebribe, so we just have to cancel
   -- bribe status
   if ai.isbribed(attacker) then
      p:setBribed( false )
   end

   -- Notify followers that we've been attacked
   if not si.fighting then
      for k,v in ipairs(p:followers()) do
         p:msg( v, "l_attacked", attacker )
      end
      local l = p:leader()
      if l then
         p:msg( l, "f_attacked", attacker )
      end
   end

   -- Generate distress if necessary
   gen_distress_attacked( attacker )
end

-- Required "attacked" function
function attacked( attacker )
   -- Ignore hits from dead pilots.
   if not attacker:exists() then
      return
   end

   local task = ai.taskname()
   local si = _stateinfo( task )

   -- Notify that pilot has been attacked before
   local p = ai.pilot()
   if not mem.attacked then
      mem.attacked = true
      mem.found_illegal = false -- We clear here so the player can't attack and still bribe
      if ai.hasfighterbays() then
         for k,v in ipairs(p:followers()) do
            p:msg( v, "e_clear" )
         end
      end
   end

   -- Pilot shouldn't be allowed to rebribe, so we just have to cancel
   -- bribe status
   if ai.isbribed(attacker) then
      p:setBribed( false )
   end

   -- Cooldown should be left running if not taking heavy damage.
   if mem.cooldown then
      local _a, pshield = p:health()
      if pshield < 90 then
         mem.cooldown = false
         p:setCooldown( false )
      else
         return
      end
   end

   -- Notify followers that we've been attacked
   if not si.fighting then
      for k,v in ipairs(p:followers()) do
         p:msg( v, "l_attacked", attacker )
      end
      local l = p:leader()
      if l then
         p:msg( l, "f_attacked", attacker )
      end
   end

   -- Generate distress if necessary
   gen_distress_attacked( attacker )

   -- If forced we'll stop after telling friends
   if si.forced then return end

   if not si.fighting then
      if mem.defensive then
         -- Some taunting
         ai.hostile(attacker) -- Should be done before taunting
         consider_taunt( attacker, false )

         -- Now pilot fights back
         clean_task( task )
         ai.pushtask("attack", attacker)
      else

         -- Runaway
         if not mem.norun then
            ai.pushtask("runaway", attacker)
         end
      end

   -- Let attacker profile handle it.
   elseif si.attack then
      atklib.attacked( attacker )

   elseif task == "runaway" then
      if ai.taskdata() ~= attacker and not mem.norun then
         ai.poptask()
         ai.pushtask("runaway", attacker)
      end
   end
end

-- Default create function just runs create_post
function create ()
   create_pre()
   create_post()
end

-- Sets up some per-pilot defaults that can be overriden afterwards
function create_pre ()
   local p        = ai.pilot()
   mem.tookoff    = p:flags("takingoff")
   mem.jumpedin   = p:flags("jumpingin")
   mem.carried    = p:flags("carried")
   mem.mothership = p:mothership()

   -- Amount of faction lost when the pilot distresses at the player
   -- Should be roughly 1 for a 20 point llama and 4.38 for a 150 point hawking
   mem.distress_hit = math.max( 0, math.pow( p:ship():points(), 0.37 )-2)

   -- Tune PD parameter (with a nice heuristic formula)
   local ps = p:stats()
   mem.Kd = math.max( 5., 10.84 * (180./ps.turn + ps.speed/ps.accel) - 10.82 )

   -- Just give some random fuel
   if p ~= player.pilot() then
      if mem.tookoff then
         p:setFuel( true ) -- Full fuel
      else
         local f = (rnd.twosigma()/4 + 0.5)*(ps.fuel_max-ps.fuel_consumption)
         f = f + ps.fuel_consumption
         p:setFuel( f )
      end
   end

   mem.elapsed = 0 -- Restart elapsed timer

   -- Choose attack algorithm
   atklib.choose()
end

-- Finishes create stuff like choose attack and prepare plans
function create_post ()
   mem.scanned    = {} -- must create for each pilot

   -- Give a small delay... except for escorts?
   if mem.jumpedin and not mem.carried then
      ai.settimer( 0, rnd.uniform(5.0, 6.0) )
      ai.pushtask("jumpin_wait")
   end

   -- Fighters give much smaller faction hits
   if mem.carried then
      mem.distress_hit = mem.distress_hit * 0.1
   end
end

-- Set transport ship parameters
function transportParam ( price )
   mem.aggressive  = false
   mem.whiteknight = false
   mem.loiter      = 0
   mem.land_planet = true
   mem.defensive   = false
   mem.enemyclose  = 500
   mem.careful     = true
   ai.setcredits( rnd.rnd(price/100, price/25) )
end

-- taunts
function taunt( _target, _offensive )
   -- Empty stub
end

-- Lower taunt frequency, at most once per X seconds per target
mem._taunted = {}
function consider_taunt( target, offensive )
   local id = target:id()
   local last_taunted = mem._taunted[id] or -100
   if mem.elapsed - last_taunted > 15 then
      taunt( target, offensive )
      mem._taunted[id] = mem.elapsed
   end
end

-- Handle distress signals
function distress( pilot, attacker )
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
   elseif mem.whiteknight and not pilot:memory().aggressive then
      badguy = attacker
   end

   -- Cannot discern the bad guy, so just look the other way
   if not badguy then return end

   local task = ai.taskname()
   local si   = _stateinfo( task )
   -- Already fighting
   if si.attack then
      -- Ignore if not interested in attacking
      if not should_attack( badguy, si, false ) then return end

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
         if not should_attack( badguy, si, false ) then return end
         if p:inrange( badguy ) then -- TODO: something to help in the other case
            clean_task( task )
            ai.pushtask( "attack", badguy )
         end
      else
         if p:inrange(badguy) and ai.dist(badguy) < mem.safe_distance and not mem.norun then
            ai.pushtask( "runaway", badguy )
         end
      end
   end
end


-- Handles generating distress messages
function gen_distress( target )
   if not mem.distress then return end

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
         mem.distressmsgfunc( target )
      else
         ai.distress( mem.distressmsg )
      end
      mem.distressed = 1
   end
end

function gen_distress_attacked( attacker )
   if not mem.distress then return end

   -- Already attacked so ignore new hits (should reset)
   if mem.attacked then return end

   if mem.distressmsgfunc then
      mem.distressmsgfunc( attacker )
   else
      ai.distress( mem.distressmsg )
   end
   mem.distressed = 1
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
   local p = ai.pilot()
   local mean = p:weapsetHeat()
   local _, pshield = p:health()

   -- Don't want to cool down again so soon.
   -- By default, 15 ticks will be 30 seconds.
   if mem.tickssincecooldown < 15 then
      return
   -- The weapons are extremely hot and cooldown should be triggered.
   -- This did not work before. However now it causes ships to just stop dead and wait for energy regen.
   -- Not sure this is better...
   elseif mean > mem.heatthreshold and pshield > 50 then
      mem.cooldown = true
      p:setCooldown(true)
   end
   if pshield == nil then
      player.msg("pshield = nil")
   end
end


-- Decide if the task is likely to become obsolete once attack is finished
function clean_task( task )
   task = task or ai.taskname()
   if task=="brake" or task=="inspect_moveto" then
      ai.poptask()
   end
end
