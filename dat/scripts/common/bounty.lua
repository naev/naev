--[[

   Framework for dead or alive bounty missions.

--]]
local pir = require "common.pirate"
local fmt = require "format"
--local pilotname = require "pilotname"
local vntk = require "vntk"
local lmisn = require "lmisn"
local lanes = require "ai.core.misc.lanes"

local bounty = {}

-- luacheck: globals board_fail bounty_setup get_faction misn_title msg pay_capture_text pay_kill_text pilot_death share_text subdue_fail_text subdue_text succeed (shared with derived missions neutral.pirbounty_alive, proteron.dissbounty_dead)

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

-- Mission details
misn_title = {
   _("Tiny Dead or Alive Bounty in {sys}"),
   _("Small Dead or Alive Bounty in {sys}"),
   _("Moderate Dead or Alive Bounty in {sys}"),
   _("High Dead or Alive Bounty in {sys}"),
   _("Dangerous Dead or Alive Bounty in {sys}"),
}

mem.misn_desc = _([[The pirate known as {pirname} was recently seen in the {sys} system. {fct} authorities want this pirate dead or alive. {pirname} is believed to be flying a {shipclass}-class ship. The pirate may disappear if you take too long to reach the {sys} system.

#nTarget:#0 {pirname} ({shipclass}-class ship)
#nWanted:#0 Dead or Alive
#nLast Seen:#0 {sys} system]])

-- Messages
msg = {
   _("{plt} got away."),
   _("Another pilot eliminated {plt}."),
   _("You have left the {sys} system."),
}
local msg_gotaway_def = _("{plt} got away.")
local msg_eliminated_other_def = _("Another pilot eliminated {plt}.")
local msg_leftsystem_def = _("You have left the {sys} system.")

local osd_title_def  = _("Bounty Hunt")
local osd_goto_def   = _("Fly to the {sys} system")
local osd_objective_def = _("Kill or capture {plt}")
local osd_reward_def = _("Land in {fct} territory to collect your bounty")

-- Non-persistent state
local hunters = {}
local hunter_hits = {}
local target_ship

function bounty.init( system, targetname, targetship, targetfaction, reward, params )
   params = params or {}

   mem._bounty = {}
   local b = mem._bounty
   b.system          = system
   b.targetname      = targetname
   b.targetship      = targetship
   b.targetfaction   = targetfaction
   b.reward          = reward
   b.reputation      = params.reputation
   -- Other important stuff
   b.targetfactionfunc = params.targetfactionfunc
   b.payingfaction   = params.payingfaction or faction.get("Independent")
   b.jumps_permitted = params.jumps_permitted or (system.cur():jumpDist(b.system) + rnd.rnd(3,5))
   b.alive_only      = params.alive_only
   -- Custom messages (can be tables of messages from which one will be chosen)
   b.msg_subdue      = params.msg_subdue or msg_subdue_def
   b.msg_kill        = params.msg_killed or msg_killed_def
   b.msg_captured    = params.msg_captured or msg_captured_def
   b.msg_shared      = params.msg_shared  or msg_shared_def
   b.msg_gotaway     = params.msg_gotaway or msg_gotaway_def
   b.msg_eliminated_other_def = params.msg_eliminated_other or msg_eliminated_other_def
   b.msg_leftsystem  = params.msg_leftsystem or msg_leftsystem_def
   -- OSD stuff
   b.osd_title       = params.osd_title or osd_title_def
   b.osd_goto        = params.osd_goto or osd_goto_def
   b.osd_objective   = params.osd_objective or osd_objective_def
   b.osd_reward      = params.osd_reward or osd_reward_def

   -- Set up mission information
   b.marker = misn.markerAdd( b.system, "computer" )
end

function bounty.accept()
   local b = mem._bounty

   misn.osdCreate( b.osd_title, {
      fmt.f( b.osd_goto,      {sys=b.system} ),
      fmt.f( b.osd_objective, {plt=b.targetname} ),
      fmt.f( b.osd_reward,    {fct=b.payingfaction} )
   } )

   b.last_sys = system.cur()
   b.job_done = false
   b.target_killed = false

   hook.jumpin( "_bounty_jumpin" )
   hook.jumpout( "_bounty_jumpout" )
   hook.takeoff( "_bounty_takeoff" )
   hook.land( "_bounty_land" )
end

local spawn_bounty
function _bounty_jumpin ()
   local b = mem._bounty

   -- Nothing to do.
   if system.cur() ~= b.system then
      return
   end

   local jmp = jump.get( system.cur(), b.last_sys )
   local L = lanes.get( get_faction(), "non-friendly")
   local m = 3e3 -- margin
   local pos
   if jmp then
      local r =  6e3
      local p = jmp:pos()
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
   spawn_bounty( pos )
end

function _bounty_jumpout ()
   local b = mem._bounty
   b.jumps_permitted = b.jumps_permitted - 1
   b.last_sys = system.cur()
   if not b.job_done and b.last_sys == b.system then
      lmisn.fail( fmt.f( msg[3], {sys=b.last_sys} ) )
   end
end

function _bounty_takeoff ()
   spawn_bounty()
end

function _bounty_land ()
   local b = mem._bounty
   b.jumps_permitted = b.jumps_permitted - 1

   local okspob = false
   -- Matching faction is always OK
   if spob.cur():faction() == b.payingfaction then
      okspob = true
   -- Special case static factions we look for non-hostiles
   elseif b.payingfaction:static() and not b.payingfaction:areEnemies(spob.cur():faction()) then
      okspob = true
   end

   if b.job_done and okspob then
      local pay_text
      if b.target_killed then
         pay_text = b.msg_killed[ rnd.rnd( 1, #b.msg_killed ) ]
      else
         pay_text = b.msg_captured[ rnd.rnd( 1, #b.msg_captured ) ]
      end
      vntk.msg( _("Mission Completed"), fmt.f( pay_text, {plt=b.targetname, credits=fmt.credits(b.reward)} ) )
      player.pay( b.reward )
      if b.reputation then
         b.payingfaction:modPlayerSingle( b.reputation )
         pir.reputationNormalMission( b.reputation )
      end
      misn.finish( true )
   end
end

function _bounty_disable ()
end

-- Succeed the mission, make the player head to a planet for pay
function succeed ()
   local b = mem._bounty

   b.job_done = true
   misn.osdActive( 3 )
   if b.marker ~= nil then
      misn.markerRm( b.marker )
   end
   hook.rm( b.pir_jump_hook )
   hook.rm( b.pir_land_hook )
end

function _bounty_board ()
   local b = mem._bounty

   player.unboard()
   local t = fmt.f( b.msg_subdue[ rnd.rnd( 1, #b.msg_subdue ) ], {plt=b.targetname} )
   vntk.msg( _("Captured Alive"), t )
   succeed()
   b.target_killed = false
   target_ship:changeAI( "dummy" )
   target_ship:setHilight( false )
   target_ship:disable() -- Stop it from coming back
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

function _bounty_death( _p, attacker )
   local b = mem._bounty

   if b.alive_only then
      lmisn.fail( fmt.f( _("{plt} has been killed."), {plt=mem.name} ) )
   end

   if attacker and attacker:withPlayer() then
      succeed()
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
         succeed()
         b.target_killed = true
      elseif player_hits >= top_hits / 2 and rnd.rnd() < 0.5 then
         b.hailer = hook.pilot( top_hunter, "hail", "hunter_hail", top_hunter )
         b.credits = b.credits * player_hits / total_hits
         hook.pilot( top_hunter, "jump", "hunter_leave" )
         hook.pilot( top_hunter, "land", "hunter_leave" )
         hook.jumpout( "hunter_leave" )
         hook.land( "hunter_leave" )
         player.msg( "#r" .. fmt.f( msg[2], {plt=b.targetname} ) .. "#0" )
         hook.timer( 3.0, "timer_hail", top_hunter )
         misn.osdDestroy()
      else
         lmisn.fail( fmt.f( msg[2], {plt=b.targetname} ) )
      end
   end
end

function _bounty_jump ()
   local b = mem._bounty
   lmisn.fail( fmt.f( msg[1], {plt=b.targetname} ) )
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

-- Adjust pirate faction (used for "alive" bounties)
local function get_faction()
   local b = mem._bounty

   if b.targetfactionfunc then
      return _G[b.targetfactionfunc]()
   end
   return b.targetfaction
end

-- Spawn the ship at the location param.
function spawn_bounty( params )
   local b = mem._bounty

   -- Not the time to spawn
   if b.job_done or system.cur()~=b.system then
      return
   end

   -- Can't jump anymore
   if b.jumps_permitted < 0 then
      lmisn.fail( fmt.f( b.msg_gotaway, {plt=b.targetname} ) )
      return
   end

   misn.osdActive( 2 )
   target_ship = pilot.add( b.targetship, get_faction(), params, b.targetname )
   local aimem = target_ship:memory()
   aimem.loiter = math.huge -- Should make them loiter forever
   target_ship:setHilight( true )
   hook.pilot( target_ship, "disable", "_bounty_disable" )
   hook.pilot( target_ship, "board", "_bounty_board" )
   hook.pilot( target_ship, "attacked", "_bounty_attacked" )
   b.death_hook = hook.pilot( target_ship, "death", "_bounty_death" )
   b.pir_jump_hook = hook.pilot( target_ship, "jump", "_bounty_jump" )
   b.pir_land_hook = hook.pilot( target_ship, "land", "_bounty_jump" )

   return target_ship
end

return bounty
