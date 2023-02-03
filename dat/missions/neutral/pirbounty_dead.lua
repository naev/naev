--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dead Or Alive Bounty">
 <priority>4</priority>
 <cond>
   return require("misn_test").mercenary()
 </cond>
 <chance>360</chance>
 <location>Computer</location>
 <faction>Dvaered</faction>
 <faction>Empire</faction>
 <faction>Frontier</faction>
 <faction>Goddard</faction>
 <faction>Independent</faction>
 <faction>Sirius</faction>
 <faction>Soromid</faction>
 <faction>Za'lek</faction>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[

   Dead or Alive Pirate Bounty

   Can work with any faction.

--]]
local pir = require "common.pirate"
local fmt = require "format"
local pilotname = require "pilotname"
local vntk = require "vntk"
local lmisn = require "lmisn"

-- luacheck: globals board_fail bounty_setup get_faction misn_title msg pay_capture_text pay_kill_text pilot_death share_text subdue_fail_text subdue_text succeed (shared with derived missions neutral.pirbounty_alive, proteron.dissbounty_dead)

subdue_text = {
   _("You and your crew infiltrate the ship's pathetic security and subdue {plt}. You transport the pirate to your ship."),
   _("Your crew has a difficult time getting past the ship's security, but eventually succeeds and subdues {plt}."),
   _("The pirate's security system turns out to be no match for your crew. You infiltrate the ship and capture {plt}."),
   _("Your crew infiltrates the pirate's ship and captures {plt}."),
   _("Getting past this ship's security was surprisingly easy. Didn't they know that {plt} was wanted?"),
}

subdue_fail_text = {
   _("Try as you might, you cannot get past the pirate's security system. Defeated, you and your crew return to the ship."),
   _("The ship's security system locks you out."),
   _("Your crew comes close to getting past the pirate's security system, but ultimately fails."),
   _("It seems your crew is no match for this ship's security system. You return to your ship."),
}

pay_kill_text = {
   _("After verifying that you killed {plt}, an officer hands you your pay of #g{credits}#0."),
   _("After verifying that {plt} is indeed dead, the tired-looking officer smiles and hands you your pay of #g{credits}#0."),
   _("The officer seems pleased that {plt} is finally dead. They thank you and promptly hand you your pay of #g{credits}#0."),
   _("The paranoid-looking officer takes you into a locked room, where the death of {plt} is quietly verified. The officer then pays you of #g{credits}#0 and sends you off."),
   _("When you ask the officer for your bounty on {plt}, they sigh, lead you into an office, go through some paperwork, and hand you your pay of #g{credits}#0, mumbling something about how useless the bounty system is."),
   _("The officer verifies the death of {plt}, goes through the necessary paperwork, and hands you your pay of #g{credits}#0, looking bored the entire time."),
}

pay_capture_text = {
   _("An officer takes {plt} into custody and hands you your pay of #g{credits}#0."),
   _("The officer seems to think your decision to capture {plt} alive was foolish. They carefully take the pirate off your hands, taking precautions you think are completely unnecessary, and then hand you your pay of #g{credits}#0."),
   _("The officer you deal with seems to especially dislike {plt}. The pirate is taken off your hands and you are handed your pay of #g{credits}#0 without a word."),
   _("A fearful-looking officer rushes {plt} into a secure hold, pays you the appropriate bounty of #g{credits}#0, and then hurries off."),
   _("The officer you greet gives you a puzzled look when you say that you captured {plt} alive. Nonetheless, they politely take the pirate off of your hands and hand you your pay of #g{credits}#0."),
}

share_text = {
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

mem.osd_title = _("Bounty Hunt")
mem.osd_msg = {
   _("Fly to the {sys} system"),
   _("Kill or capture {plt}"),
   _("Land in {fct} territory to collect your bounty"),
}

-- Non-persistent state
local hunters = {}
local hunter_hits = {}
local target_ship

local spawn_pirate -- Forward-declared functions


function create ()
   mem.paying_faction = spob.cur():faction()

   local systems = lmisn.getSysAtDistance( system.cur(), 1, 3,
      function(s)
         return pir.systemPresence( s ) > 0
      end )

   if #systems == 0 then
      -- No pirates nearby
      misn.finish( false )
   end

   mem.missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( mem.missys, false, true ) then misn.finish( false ) end

   mem.jumps_permitted = system.cur():jumpDist(mem.missys) + rnd.rnd(3,5)

   local num_pirates = pir.systemPresence( mem.missys )
   if num_pirates <= 50 then
      mem.level = 1
   elseif num_pirates <= 100 then
      mem.level = rnd.rnd( 1, 2 )
   elseif num_pirates <= 200 then
      mem.level = rnd.rnd( 2, 3 )
   elseif num_pirates <= 300 then
      mem.level = rnd.rnd( 3, 4 )
   else
      mem.level = rnd.rnd( 4, 5 )
   end

   -- Pirate details
   mem.name = pilotname.pirate()
   mem.pship = "Pirate Hyena"
   mem.credits = 50e3
   mem.reputation = 0
   mem.board_failed = false
   mem.pship, mem.credits, mem.reputation = bounty_setup()

   -- Faction prefix
   local prefix
   if mem.paying_faction:static() then
      prefix = ""
   else
      prefix = require("common.prefix").prefix(mem.paying_faction)
   end

   -- Set mission details
   misn.setTitle( prefix..fmt.f(misn_title[mem.level], {sys=mem.missys}) )
   local desc = fmt.f(mem.misn_desc,
      {pirname=mem.name, sys=mem.missys, fct=mem.paying_faction, shipclass=_(ship.get(mem.pship):classDisplay()) })
   if not mem.paying_faction:static() then
      desc = desc.."\n"..fmt.f(_([[#nReputation Gained:#0 {fct}]]),
         {fct=mem.paying_faction})
   end
   misn.setDesc(desc)
   misn.setReward( fmt.credits( mem.credits ) )
   mem.marker = misn.markerAdd( mem.missys, "computer" )
end


function accept ()
   misn.accept()

   mem.osd_msg[1] = fmt.f( mem.osd_msg[1], {sys=mem.missys} )
   mem.osd_msg[2] = fmt.f( mem.osd_msg[2], {plt=mem.name} )
   mem.osd_msg[3] = fmt.f( mem.osd_msg[3], {fct=mem.paying_faction} )
   misn.osdCreate( mem.osd_title, mem.osd_msg )

   mem.last_sys = system.cur()
   mem.job_done = false
   mem.target_killed = false

   hook.jumpin( "jumpin" )
   hook.jumpout( "jumpout" )
   hook.takeoff( "takeoff" )
   hook.land( "land" )
end


function jumpin ()
   -- Nothing to do.
   if system.cur() ~= mem.missys then
      return
   end

   local pos = jump.pos( system.cur(), mem.last_sys )
   local offset_ranges = { { -2500, -1500 }, { 1500, 2500 } }
   local xrange = offset_ranges[ rnd.rnd( 1, #offset_ranges ) ]
   local yrange = offset_ranges[ rnd.rnd( 1, #offset_ranges ) ]
   pos = pos + vec2.new( rnd.rnd( xrange[1], xrange[2] ), rnd.rnd( yrange[1], yrange[2] ) )
   spawn_pirate( pos )
end


function jumpout ()
   mem.jumps_permitted = mem.jumps_permitted - 1
   mem.last_sys = system.cur()
   if not mem.job_done and mem.last_sys == mem.missys then
      lmisn.fail( fmt.f( msg[3], {sys=mem.last_sys} ) )
   end
end


function takeoff ()
   spawn_pirate()
end


function land ()
   mem.jumps_permitted = mem.jumps_permitted - 1
   if mem.job_done and spob.cur():faction() == mem.paying_faction then
      local pay_text
      if mem.target_killed then
         pay_text = pay_kill_text[ rnd.rnd( 1, #pay_kill_text ) ]
      else
         pay_text = pay_capture_text[ rnd.rnd( 1, #pay_capture_text ) ]
      end
      vntk.msg( _("Mission Completed"), fmt.f( pay_text, {plt=mem.name, credits=fmt.credits(mem.credits)} ) )
      player.pay( mem.credits )
      mem.paying_faction:modPlayerSingle( mem.reputation )
      pir.reputationNormalMission( mem.reputation )
      misn.finish( true )
   end
end


function pilot_disable ()
end


function pilot_board ()
   player.unboard()
   if mem.can_capture then
      local t = fmt.f( subdue_text[ rnd.rnd( 1, #subdue_text ) ], {plt=mem.name} )
      vntk.msg( _("Captured Alive"), t )
      succeed()
      mem.target_killed = false
      target_ship:changeAI( "dummy" )
      target_ship:setHilight( false )
      target_ship:disable() -- Stop it from coming back
      hook.rm( mem.death_hook )
   else
      local t = fmt.f( subdue_fail_text[ rnd.rnd( 1, #subdue_fail_text ) ], {plt=mem.name} )
      vntk.msg( _("Capture Failed"), t )
      board_fail()
   end
end


function pilot_attacked( _p, attacker, dmg )
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


function pilot_death( _p, attacker )
   if attacker and attacker:withPlayer() then
      succeed()
      mem.target_killed = true
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
         mem.target_killed = true
      elseif player_hits >= top_hits / 2 and rnd.rnd() < 0.5 then
         mem.hailer = hook.pilot( top_hunter, "hail", "hunter_hail", top_hunter )
         mem.credits = mem.credits * player_hits / total_hits
         hook.pilot( top_hunter, "jump", "hunter_leave" )
         hook.pilot( top_hunter, "land", "hunter_leave" )
         hook.jumpout( "hunter_leave" )
         hook.land( "hunter_leave" )
         player.msg( "#r" .. fmt.f( msg[2], {plt=mem.name} ) .. "#0" )
         hook.timer( 3.0, "timer_hail", top_hunter )
         misn.osdDestroy()
      else
         lmisn.fail( fmt.f( msg[2], {plt=mem.name} ) )
      end
   end
end


function pilot_jump ()
   lmisn.fail( fmt.f( msg[1], {plt=mem.name} ) )
end


function timer_hail( arg )
   if arg ~= nil and arg:exists() then
      arg:hailPlayer()
   end
end


function hunter_hail( _arg )
   hook.rm( mem.hailer )
   player.commClose()

   local text = share_text[ rnd.rnd( 1, #share_text ) ]
   vntk.msg( _("A Smaller Reward"), fmt.f( text, {plt=mem.name} ) )

   player.pay( mem.credits )
   misn.finish( true )
end


function hunter_leave ()
   misn.finish( false )
end


-- Set up the ship, credits, and reputation based on the level.
function bounty_setup ()
   local pship, credits, reputation
   if mem.level == 1 then
      if rnd.rnd() < 0.5 then
         pship = "Pirate Hyena"
         credits = 80e3 + rnd.sigma() * 15e3
      else
         pship = "Pirate Shark"
         credits = 100e3 + rnd.sigma() * 30e3
      end
      reputation = 0.5
   elseif mem.level == 2 then
      if rnd.rnd() < 0.5 then
         pship = "Pirate Vendetta"
      else
         pship = "Pirate Ancestor"
      end
      credits = 300e3 + rnd.sigma() * 50e3
      reputation = 1
   elseif mem.level == 3 then
      if rnd.rnd() < 0.5 then
         pship = "Pirate Admonisher"
      else
         pship = "Pirate Phalanx"
      end
      credits = 500e3 + rnd.sigma() * 80e3
      reputation = 2
   elseif mem.level == 4 then
      if rnd.rnd() < 0.5 then
         pship = "Pirate Starbridge"
      else
         pship = "Pirate Rhino"
      end
      credits = 700e3 + rnd.sigma() * 90e3
      reputation = 2.8
   elseif mem.level == 5 then
      pship = "Pirate Kestrel"
      credits = 1e6 + rnd.sigma() * 100e3
      reputation = 3.5
   end
   return pship, credits, reputation
end


-- Spawn the ship at the location param.
function spawn_pirate( param )
   -- Not the time to spawn
   if mem.job_done or system.cur() ~= mem.missys then
      return
   end

   -- Can't jump anymore
   if mem.jumps_permitted < 0 then
      lmisn.fail( fmt.f( msg[1], {plt=mem.name} ) )
      return
   end

   misn.osdActive( 2 )
   target_ship = pilot.add( mem.pship, get_faction(), param, mem.name )
   local aimem = target_ship:memory()
   aimem.loiter = math.huge -- Should make them loiter forever
   target_ship:setHilight( true )
   hook.pilot( target_ship, "disable", "pilot_disable" )
   hook.pilot( target_ship, "board", "pilot_board" )
   hook.pilot( target_ship, "attacked", "pilot_attacked" )
   mem.death_hook = hook.pilot( target_ship, "death", "pilot_death" )
   mem.pir_jump_hook = hook.pilot( target_ship, "jump", "pilot_jump" )
   mem.pir_land_hook = hook.pilot( target_ship, "land", "pilot_jump" )

   --[[
   local pir_crew = target_ship:stats().crew
   local pl_crew = player.pilot():stats().crew
   if rnd.rnd() > (0.5 * (10 + pir_crew) / (10 + pl_crew)) then
      mem.can_capture = true
   else
      mem.can_capture = false
   end
   --]]
   -- Disabling and boarding is hard enough as is to randomly fail
   -- TODO potentially do a small capturing minigame here
   mem.can_capture = true

   return target_ship
end


-- Adjust pirate faction (used for "alive" bounties)
function get_faction()
   return faction.dynAdd( "Pirate", "Wanted Pirate", _("Wanted Pirate"), {clear_enemies=true, clear_allies=true} )
end


-- Fail to board the ship; must kill the target instead
-- (Unused in this mission; used by pirbounty_alive)
function board_fail ()
   mem.board_failed = true
end


-- Succeed the mission, make the player head to a planet for pay
function succeed ()
   mem.job_done = true
   misn.osdActive( 3 )
   if mem.marker ~= nil then
      misn.markerRm( mem.marker )
   end
   hook.rm( mem.pir_jump_hook )
   hook.rm( mem.pir_land_hook )
end
