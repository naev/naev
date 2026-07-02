--[[

   Framework for dead or alive bounty missions.

   TODO: support for needing to defeat a % of target ships
--]]
local pir = require "common.pirate"
local fmt = require "format"
--local pilotname = require "pilotname"
local vntk = require "vntk"
local lmisn = require "lmisn"
local lanes = require "ai.core.misc.lanes"

local bounty = {}

local msg_subdue_def = {
   _("You and your crew infiltrate the ship's pathetic security and subdue {plt}. You transport the pirate to your ship."),
   _("Your crew has a difficult time getting past the ship's security, but eventually succeeds and subdues {plt}."),
   _("The pirate's security system turns out to be no match for your crew. You infiltrate the ship and capture {plt}."),
   _("Your crew infiltrates the pirate's ship and captures {plt}."),
   _("Getting past this ship's security was surprisingly easy. Didn't they know that {plt} was wanted?"),
}

local msg_killed_def = {
   _("After verifying that you killed {plt}, an officer hands you your pay of #g{credits}#0."),
   _("After verifying that {plt} is indeed dead, the tired-looking officer smiles and hands you your pay of #g{credits}#0."),
   _("The officer seems pleased that {plt} is finally dead. They thank you and promptly hand you your pay of #g{credits}#0."),
   _("The paranoid-looking officer takes you into a locked room, where the death of {plt} is quietly verified. The officer then pays you of #g{credits}#0 and sends you off."),
   _("When you ask the officer for your bounty on {plt}, they sigh, lead you into an office, go through some paperwork, and hand you your pay of #g{credits}#0, mumbling something about how useless the bounty system is."),
   _("The officer verifies the death of {plt}, goes through the necessary paperwork, and hands you your pay of #g{credits}#0, looking bored the entire time."),
}

local msg_captured_def = {
   _("An officer takes {plt} into custody and hands you your pay of #g{credits}#0."),
   _("The officer seems to think your decision to capture {plt} alive was foolish. They carefully take the pirate off your hands, taking precautions you think are completely unnecessary, and then hand you your pay of #g{credits}#0."),
   _("The officer you deal with seems to especially dislike {plt}. The pirate is taken off your hands and you are handed your pay of #g{credits}#0 without a word."),
   _("A fearful-looking officer rushes {plt} into a secure hold, pays you the appropriate bounty of #g{credits}#0, and then hurries off."),
   _("The officer you greet gives you a puzzled look when you say that you captured {plt} alive. Nonetheless, they politely take the pirate off of your hands and hand you your pay of #g{credits}#0."),
}

local msg_shared_def = {
   _([["Greetings. I can see that you were trying to collect a bounty on {plt}. Well, as you can see, I earned the bounty, but I don't think I would have succeeded without your help, so I've transferred a portion of the bounty into your account."]]),
   _([["Sorry about getting in the way of your bounty. I don't really care too much about the money, but I just wanted to make sure the galaxy would be rid of that scum; I've seen the villainy of {plt} first-hand, you see. So, as an apology, I would like to offer you the portion of the bounty you clearly earned. The money will be in your account shortly."]]),
   _([["Hey, thanks for the help back there. I don't know if I would have been able to handle {plt} alone! Anyway, since you were such a big help, I have transferred what I think is your fair share of the bounty to your bank account."]]),
   _([["Heh, thanks! I think I would have been able to take out {plt} by myself, but still, I appreciate your assistance. Here, I'll transfer some of the bounty to you, as a token of my appreciation."]]),
   _([["Ha ha ha, looks like I beat you to it this time, eh? Well, I don't do this often, but here, have some of the bounty. I think you deserve it."]]),
}

mem.misn_desc = _([[The pirate known as {pirname} was recently seen in the {sys} system. {fct} authorities want this pirate dead or alive. {pirname} is believed to be flying a {shipclass}-class ship. The pirate may disappear if you take too long to reach the {sys} system.

#nTarget:#0 {pirname} ({shipclass}-class ship)
#nWanted:#0 Dead or Alive
#nLast seen:#0 {sys} system]])

-- Messages
local msg_gotaway_def = _("{plt} got away.")
local msg_eliminated_other_def = _("Another pilot eliminated {plt}.")
local msg_leftsystem_def = _("You have left the {sys} system.")

local osd_title_def  = _("Bounty Hunt")
local osd_goto_def   = _("Fly to the {sys} system before {time_limit} ({time} remaining)")
local osd_goto_nodeadline_def   = _("Fly to the {sys} system")
local osd_objective_def = _("Kill or capture {plt}")
local osd_objective_capture_def = _("Disable and capture {plt}")
local osd_reward_def = _("Land in {fct} territory to collect your bounty")
local osd_reward_static_def = _("Land to collect your bounty")

-- Non-persistent state
local hunters = {}
local hunter_hits = {}
local target_ship

function bounty.init( system, targetname, targetship, reward, params )
   params = params or {}

   mem._bounty = {}
   local b = mem._bounty
   b.trackingvar     = params.trackingvar
   b.curspb          = spob.cur()
   b.system          = system
   b.targetname      = targetname
   b.targetship      = targetship
   if type(b.targetship) ~= "table" then
      b.targetship   = {b.targetship}
   end
   b.reward          = reward
   b.reputation      = params.reputation
   -- Other important stuff
   b.targetfaction   = params.targetfaction
   b.targetfactionfunc = params.targetfactionfunc
   b.payingfaction   = params.payingfaction or faction.get("Independent")
   b.deadline        = params.deadline
   b.alive_only      = params.alive_only
   b.spawnfunc       = params.spawnfunc
   b.boardfunc       = params.boardfunc
   b.deathfunc       = params.deathfunc
   b.completefunc    = params.completefunc
   b.staticfaction   = params.staticfaction
   -- Custom messages (can be tables of messages from which one will be chosen)
   b.msg_subdue      = params.msg_subdue or msg_subdue_def
   b.msg_killed      = params.msg_killed or msg_killed_def
   b.msg_captured    = params.msg_captured or msg_captured_def
   b.msg_shared      = params.msg_shared  or msg_shared_def
   b.msg_gotaway     = params.msg_gotaway or msg_gotaway_def
   b.msg_eliminated_other = params.msg_eliminated_other or msg_eliminated_other_def
   b.msg_leftsystem  = params.msg_leftsystem or msg_leftsystem_def
   -- OSD stuff
   b.osd_title       = params.osd_title or osd_title_def
   b.osd_goto        = params.osd_goto
   if b.osd_goto==nil then
      if b.deadline then
         b.osd_goto = osd_goto_def
      else
         b.osd_goto = osd_goto_nodeadline_def
      end
   end
   b.osd_objective   = params.osd_objective
   if not b.osd_objective_def then
      if b.alive_only then
         b.osd_objective =  osd_objective_capture_def
      else
         b.osd_objective =  osd_objective_def
      end
   end
   if params.osd_reward ~= nil then
      b.osd_erward = params.osd_reward
   else
      b.osd_reward = (b.payingfaction:static() and osd_reward_static_def) or osd_reward_def
   end

   -- Set up mission information
   b.marker = misn.markerAdd( b.system, "computer" )
end

local function update_osd ()
   local b = mem._bounty
   if b.deadline then
      local active = misn.osdGetActive() or 1
      -- Only care if first is selected, or time is ignored
      if active==1 then
         local objective = {
            fmt.f( b.osd_goto,      {sys=b.system, time_limit=b.deadline, time=(b.deadline-time.cur())} ),
            fmt.f( b.osd_objective, {plt=b.targetname} ),
         }
         if b.osd_reward then
            table.insert( objective, fmt.f( b.osd_reward, {fct=b.payingfaction} ) )
         end
         misn.osdCreate( b.osd_title, objective )
      end
   else
      local objective = {
         fmt.f( b.osd_goto,      {sys=b.system} ),
         fmt.f( b.osd_objective, {plt=b.targetname} ),
      }
      if b.osd_reward then
         table.insert( objective, fmt.f( b.osd_reward, {fct=b.payingfaction} ) )
      end
      misn.osdCreate( b.osd_title, objective )
   end
end

function bounty.accept()
   local b = mem._bounty
   misn.accept()

   update_osd()

   if player.isLanded() then
      b.last_spob = spob.cur()
   else
      b.last_sys = system.cur()
   end
   b.job_done = false
   b.target_killed = false

   hook.jumpin( "_bounty_jumpin" )
   hook.jumpout( "_bounty_jumpout" )
   hook.takeoff( "_bounty_takeoff" )
   hook.land( "_bounty_land" )
   if b.deadline then
      hook.date( time.new( 0, 0, 1e3 ), "_bounty_date" )
   end
end

function _bounty_date ()
   local b = mem._bounty
   if system.cur() ~= b.system and not b.job_done then
      if time.cur() > b.deadline then
         return lmisn.fail( fmt.f(_("{plt} got away."), {plt=b.targetname} ))
      end
      update_osd()
   end
end

-- Adjust pirate faction (used for "alive" bounties)
function bounty.get_faction()
   local b = mem._bounty
   if b.targetfactionfunc then
      return _G[b.targetfactionfunc]()
   elseif not b.staticfaction then
      -- Create a dynamic faction
      local fct = faction.get(b.targetfaction)
      return faction.dynAdd( fct, "bounty_"..fct:nameRaw(), fct:name(), {clear_enemies=true, clear_allies=true} )
   end
   return b.targetfaction
end

function bounty.choose_spawn_pos()
   local b = mem._bounty
   -- Try to find a good location
   local pos
   local m = 3e3 -- margin
   local L = lanes.get( bounty.get_faction(), "non-friendly")
   if b.last_sys then
      local jmp = jump.get( system.cur(), b.last_sys )
      if jmp then
         local r =  6e3
         local p = jmp:pos()
         pos = lanes.getNonPoint( L, p, r, m )
      end
   elseif b.last_spob then
      local r =  6e3
      local p = b.last_spob:pos()
      pos = lanes.getNonPoint( L, p, r, m )
   end
   if not pos then
      local r = system.cur():radius() * 0.8
      local p = vec2.newP( rnd.rnd() * r, rnd.angle() )
      pos = lanes.getNonPoint( L, p, r, m )
   end
   if not pos then
      local r = system.cur():radius() * 0.8
      pos = vec2.newP( rnd.rnd() * r, rnd.angle() )
   end
   return pos
end

local spawn_bounty
local function _bounty_spawn ()
   local b = mem._bounty
   -- Nothing to do.
   if system.cur() ~= b.system then
      return
   end
   spawn_bounty( bounty.choose_spawn_pos() )
end
function _bounty_jumpin ()
   _bounty_spawn()
end

function _bounty_jumpout ()
   local b = mem._bounty
   b.last_sys = system.cur()
   b.last_spob = nil
   if not b.job_done and b.last_sys == b.system then
      lmisn.fail( fmt.f( b.msg_leftsystem, {sys=b.last_sys} ) )
   end
end

function _bounty_takeoff ()
   _bounty_spawn()
end

function _bounty_land ()
   local b = mem._bounty
   if not b.job_done or b.completed then return end

   b.last_sys = nil
   b.last_spob = spob.cur()

   local fct = b.payingfaction
   local spbfct = spob.cur():faction()
   if spbfct == nil then return end
   if not spob.cur():services().inhabited then return end

   local okspob = false
   -- Matching faction is always OK
   if spbfct == fct then
      okspob = true
   -- Special case static factions we look for non-hostiles
   elseif fct:static() and not fct:areEnemies(spbfct) then
      if fct:tags().generic then
         okspob = spbfct:tags().generic
      else
         okspob = true
      end
   end
   if not okspob then return end

   -- Allow custom functions
   if b.completefunc then
      b.completed = true
      if not _G[b.completefunc]() then return end
   end

   local pay_text
   if b.target_killed then
      pay_text = b.msg_killed[ rnd.rnd( 1, #b.msg_killed ) ]
   else
      pay_text = b.msg_captured[ rnd.rnd( 1, #b.msg_captured ) ]
   end
   lmisn.sfxMoney()
   vntk.msg( _("Mission Completed"), fmt.f( pay_text, {plt=b.targetname, credits=fmt.credits(b.reward)} ) )
   player.pay( b.reward )
   if b.reputation then
      b.payingfaction:hit( b.reputation )
      pir.reputationNormalMission( b.reputation )
   end
   if b.trackingvar then
      local v = var.peek( b.trackingvar[1] ) or 0
      var.push( b.trackingvar[1], v+b.trackingvar[2] )
   end
   misn.finish( true )
end

-- Succeed the mission, make the player head to a planet for pay
local function _succeed ()
   local b = mem._bounty

   b.job_done = true
   misn.osdActive( 3 )
   if b.marker ~= nil then
      misn.markerRm( b.marker )
   end
   hook.rm( b.jump_hook )
   hook.rm( b.land_hook )
end

function _bounty_board( p )
   local b = mem._bounty

   if b.boardfunc then
      if not _G[b.boardfunc]( b, p ) then return end
   end

   local pltc = commodity.new( b.targetname, _("A wanted individual captured alive.") )
   local t = fmt.f( b.msg_subdue[ rnd.rnd( 1, #b.msg_subdue ) ], {plt=b.targetname} )
   vntk.msg( _("Captured Alive"), t )
   _succeed()
   b.target_killed = false
   misn.cargoAdd( pltc, 0 )
   target_ship:setHilight( false )
   target_ship:setDisable() -- Stop it from coming back
   hook.rm( b.death_hook )
end

function _bounty_attacked( _p, attacker, dmg )
   if attacker == nil then
      return
   end
   local found = false

   for i, j in ipairs( hunters ) do
      if j == attacker then
         hunter_hits[i] = hunter_hits[i] + dmg
         found = true
      end
   end

   if not found then
      local i = #hunters + 1
      hunters[i] = attacker
      hunter_hits[i] = dmg
   end
end

function _bounty_death( p, attacker )
   local b = mem._bounty

   if b.alive_only then
      lmisn.fail( fmt.f( _("{plt} has been killed."), {plt=b.targetname} ) )
   end

   if b.deathfunc then
      if not _G[b.deathfunc]( b, p, attacker ) then return end
   end

   if attacker and attacker:withPlayer() then
      _succeed()
      b.target_killed = true
   else
      local top_hunter = nil
      local top_hits = 0
      local player_hits = 0
      local total_hits = 0
      for i, j in ipairs( hunters ) do
         total_hits = total_hits + hunter_hits[i]
         if j ~= nil and j:exists() then
            if j:withPlayer() then
               player_hits = player_hits + hunter_hits[i]
            elseif hunter_hits[i] > top_hits then
               top_hunter = j
               top_hits = hunter_hits[i]
            end
         end
      end

      if top_hunter == nil or player_hits >= top_hits then
         _succeed()
         b.target_killed = true
      elseif player_hits >= top_hits / 2 and rnd.rnd() < 0.5 then
         b.hailer = hook.pilot( top_hunter, "hail", "hunter_hail", top_hunter )
         b.credits = b.credits * player_hits / total_hits
         hook.pilot( top_hunter, "jump", "hunter_leave" )
         hook.pilot( top_hunter, "land", "hunter_leave" )
         hook.jumpout( "hunter_leave" )
         hook.land( "hunter_leave" )
         player.msg( "#r" .. fmt.f( b.msg_eliminated_other, {plt=b.targetname} ) .. "#0" )
         hook.timer( 3.0, "timer_hail", top_hunter )
         misn.osdDestroy()
      else
         lmisn.fail( fmt.f( b.msg_eliminated_other, {plt=b.targetname} ) )
      end
   end
end

function _bounty_jump ()
   local b = mem._bounty
   lmisn.fail( fmt.f( b.msg_gotaway, {plt=b.targetname} ) )
end

function timer_hail( arg )
   if arg ~= nil and arg:exists() then
      arg:hailPlayer()
   end
end

function hunter_hail( _arg )
   local b = mem._bounty

   hook.rm( b.hailer )
   player.commClose()

   local msg = b.msg_shared[ rnd.rnd( 1, #b.msg_shared ) ]
   vntk.msg( _("A Smaller Reward"), fmt.f( msg, {plt=b.targetname} ) )

   player.pay( b.reward )
   misn.finish( true )
end

function hunter_leave ()
   misn.finish( false )
end

-- Set up the ship, credits, and reputation based on the level.

-- Spawn the ship at the location param.
function spawn_bounty( params )
   local b = mem._bounty

   -- Not the time to spawn
   if b.job_done or system.cur()~=b.system then
      return
   end

   -- If using a spawn function
   target_ship = nil
   if b.spawnfunc then
      target_ship = _G[b.spawnfunc]( b, params )
      if type(target_ship)=="table" then
         -- TODO make it so the escorts have to be destroyed in some cases
         for k,e in ipairs(target_ship) do
            e:setHostile( true )
         end
         target_ship = target_ship[1]
      end
   else
      local fct = bounty.get_faction()
      for k,s in ipairs(b.targetship) do
         local p = pilot.add( s, fct, params )
         p:setHostile(true)
         local aimem = p:memory()
         aimem.defensive   = true -- Always try to be defensive
         aimem.loiter      = math.huge -- Should make them loiter forever
         aimem.capturable  = true
         if not target_ship then
            target_ship = p
            p:rename( b.targetname )
            -- Make esaier to spot but not fight
            p:intrinsicSet( "ew_detected", 50 )
         else
            p:setLeader( target_ship )
         end
      end
   end

   misn.osdActive( 2 )
   target_ship:setHilight( true )
   target_ship:setHostile( true )
   hook.pilot( target_ship, "board", "_bounty_board" )
   hook.pilot( target_ship, "attacked", "_bounty_attacked" )
   b.death_hook = hook.pilot( target_ship, "death", "_bounty_death" )
   b.jump_hook = hook.pilot( target_ship, "jump", "_bounty_jump" )
   b.land_hook = hook.pilot( target_ship, "land", "_bounty_jump" )

   return target_ship
end


function bounty.fleet_points( fleet )
   local points = 0
   for k,s in ipairs(fleet) do
      points = points + s:points()
   end
   return points
end

-- Tries to find a set of ships given a number of points
function bounty.choose_ships_from_points( shiplist, points )
   -- Candidate ships
   local maybeship = {}
   local maybecap = {}
   for k,v in ipairs(shiplist) do
      local p = v:points()
      if p < points then
         table.insert( maybeship, v )
         if p > points*0.5 then
            table.insert( maybecap, v )
         end
      end
   end
   if #maybeship <= 0 then
      table.sort( shiplist, function( a, b ) return a:points() < b:points() end )
      return {shiplist[1]}
   end
   table.sort( maybeship, function( a, b ) return a:points() > b:points() end )

   -- Force top three ships to be considered capitals
   if #maybecap <= 0 then
      maybecap = { maybeship[1], maybeship[2], maybeship[3] }
   end

   -- Choose capship
   local cap = maybecap[ rnd.rnd(#maybecap) ]
   points = points - cap:points()
   local smallest = maybeship[ #maybeship ]:points()
   if points < smallest then return {cap} end

   -- Must be smaller than capship
   local cappoints = cap:points()
   local newships = {}
   for k,s in ipairs(maybeship) do
      if s:points() < cappoints then
         table.insert( newships, s )
      end
   end
   maybeship = newships

   -- Other ships have to be smaller
   local ships = {cap}
   while points >= smallest do
      local candidates = rnd.permutation( maybeship )
      local s
      local id = 1
      repeat
         s = candidates[id]
         if not s then return ships end
         id = id+1
      until s:points() < points
      table.insert( ships, s )
      points = points - s:points()
   end

   return ships
end

return bounty
