--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dead Or Alive Bounty">
 <avail>
  <priority>4</priority>
  <cond>player.numOutfit("Mercenary License") &gt; 0</cond>
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
 </avail>
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

subdue_text    = {}
subdue_text[1] = _("You and your crew infiltrate the ship's pathetic security and subdue {plt}. You transport the pirate to your ship.")
subdue_text[2] = _("Your crew has a difficult time getting past the ship's security, but eventually succeeds and subdues {plt}.")
subdue_text[3] = _("The pirate's security system turns out to be no match for your crew. You infiltrate the ship and capture {plt}.")
subdue_text[4] = _("Your crew infiltrates the pirate's ship and captures {plt}.")
subdue_text[5] = _("Getting past this ship's security was surprisingly easy. Didn't they know that {plt} was wanted?")

subdue_fail_text    = {}
subdue_fail_text[1] = _("Try as you might, you cannot get past the pirate's security system. Defeated, you and your crew return to the ship.")
subdue_fail_text[2] = _("The ship's security system locks you out.")
subdue_fail_text[3] = _("Your crew comes close to getting past the pirate's security system, but ultimately fails.")
subdue_fail_text[4] = _("It seems your crew is no match for this ship's security system. You return to your ship.")

pay_kill_text    = {}
pay_kill_text[1] = _("After verifying that you killed {plt}, an officer hands you your pay of #g{credits}#0.")
pay_kill_text[2] = _("After verifying that {plt} is indeed dead, the tired-looking officer smiles and hands you your pay of #g{credits}#0.")
pay_kill_text[3] = _("The officer seems pleased that {plt} is finally dead. They thank you and promptly hand you your pay of #g{credits}#0.")
pay_kill_text[4] = _("The paranoid-looking officer takes you into a locked room, where the death of {plt} is quietly verified. The officer then pays you of #g{credits}#0 and sends you off.")
pay_kill_text[5] = _("When you ask the officer for your bounty on {plt}, they sigh, lead you into an office, go through some paperwork, and hand you your pay of #g{credits}#0, mumbling something about how useless the bounty system is.")
pay_kill_text[6] = _("The officer verifies the death of {plt}, goes through the necessary paperwork, and hands you your pay of #g{credits}#0, looking bored the entire time.")

pay_capture_text    = {}
pay_capture_text[1] = _("An officer takes {plt} into custody and hands you your pay of #g{credits}#0.")
pay_capture_text[2] = _("The officer seems to think your decision to capture {plt} alive was foolish. They carefully take the pirate off your hands, taking precautions you think are completely unnecessary, and then hand you your pay of #g{credits}#0.")
pay_capture_text[3] = _("The officer you deal with seems to especially dislike {plt}. The pirate is taken off your hands and you are handed your pay of #g{credits}#0 without a word.")
pay_capture_text[4] = _("A fearful-looking officer rushes {plt} into a secure hold, pays you the appropriate bounty of #g{credits}#0, and then hurries off.")
pay_capture_text[5] = _("The officer you greet gives you a puzzled look when you say that you captured {plt} alive. Nonetheless, they politely take the pirate off of your hands and hand you your pay of #g{credits}#0.")

share_text    = {}
share_text[1] = _([["Greetings. I can see that you were trying to collect a bounty on {plt}. Well, as you can see, I earned the bounty, but I don't think I would have succeeded without your help, so I've transferred a portion of the bounty into your account."]])
share_text[2] = _([["Sorry about getting in the way of your bounty. I don't really care too much about the money, but I just wanted to make sure the galaxy would be rid of that scum; I've seen the villainy of {plt} first-hand, you see. So as an apology, I would like to offer you the portion of the bounty you clearly earned. The money will be in your account shortly."]])
share_text[3] = _([["Hey, thanks for the help back there. I don't know if I would have been able to handle {plt} alone! Anyway, since you were such a big help, I have transferred what I think is your fair share of the bounty to your bank account."]])
share_text[4] = _([["Heh, thanks! I think I would have been able to take out {plt} by myself, but still, I appreciate your assistance. Here, I'll transfer some of the bounty to you, as a token of my appreciation."]])
share_text[5] = _([["Ha ha ha, looks like I beat you to it this time, eh? Well, I don't do this often, but here, have some of the bounty. I think you deserve it."]])

-- Mission details
misn_title = {}
misn_title[1] = _("Tiny Dead or Alive Bounty in {sys}")
misn_title[2] = _("Small Dead or Alive Bounty in {sys}")
misn_title[3] = _("Moderate Dead or Alive Bounty in {sys}")
misn_title[4] = _("High Dead or Alive Bounty in {sys}")
misn_title[5] = _("Dangerous Dead or Alive Bounty in {sys}")
misn_desc   = _("The pirate known as {pirname} was recently seen in the {sys} system. {fct} authorities want this pirate dead or alive. {pirname} is believed to be flying a {shipclass}-class ship.")

-- Messages
msg    = {}
msg[1] = _("MISSION FAILURE! {plt} got away.")
msg[2] = _("MISSION FAILURE! Another pilot eliminated {plt}.")
msg[3] = _("MISSION FAILURE! You have left the {sys} system.")

osd_title = _("Bounty Hunt")
osd_msg    = {}
osd_msg[1] = _("Fly to the {sys} system")
osd_msg[2] = _("Kill or capture {plt}")
osd_msg[3] = _("Land in {fct} territory to collect your bounty")
osd_msg["__save"] = true


hunters = {}
hunter_hits = {}


function create ()
   paying_faction = planet.cur():faction()

   local systems = lmisn.getSysAtDistance( system.cur(), 1, 3,
      function(s)
         return pir.systemPresence( s ) > 0
      end )

   if #systems == 0 then
      -- No pirates nearby
      misn.finish( false )
   end

   missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( missys ) then misn.finish( false ) end

   jumps_permitted = system.cur():jumpDist(missys) + rnd.rnd( 5 )
   if rnd.rnd() < 0.05 then
      jumps_permitted = jumps_permitted - 1
   end

   local num_pirates = pir.systemPresence( missys )
   if num_pirates <= 50 then
      level = 1
   elseif num_pirates <= 100 then
      level = rnd.rnd( 1, 2 )
   elseif num_pirates <= 200 then
      level = rnd.rnd( 2, 3 )
   elseif num_pirates <= 300 then
      level = rnd.rnd( 3, 4 )
   else
      level = rnd.rnd( 4, 5 )
   end

   -- Pirate details
   name = pilotname.pirate()
   pship = "Hyena"
   credits = 50e3
   reputation = 0
   board_failed = false
   pship, credits, reputation = bounty_setup()

   -- Set mission details
   misn.setTitle( fmt.f( misn_title[level], {sys=missys} ) )
   misn.setDesc( fmt.f( misn_desc, {pirname=name, sys=missys, fct=paying_faction, shipclass=_(ship.get(pship):classDisplay()) } ) )
   misn.setReward( fmt.credits( credits ) )
   marker = misn.markerAdd( missys, "computer" )
end


function accept ()
   misn.accept()

   osd_msg[1] = fmt.f( osd_msg[1], {sys=missys} )
   osd_msg[2] = fmt.f( osd_msg[2], {plt=name} )
   osd_msg[3] = fmt.f( osd_msg[3], {fct=paying_faction} )
   misn.osdCreate( osd_title, osd_msg )

   last_sys = system.cur()
   job_done = false
   target_killed = false

   hook.jumpin( "jumpin" )
   hook.jumpout( "jumpout" )
   hook.takeoff( "takeoff" )
   hook.land( "land" )
end


function jumpin ()
   -- Nothing to do.
   if system.cur() ~= missys then
      return
   end

   local pos = jump.pos( system.cur(), last_sys )
   local offset_ranges = { { -2500, -1500 }, { 1500, 2500 } }
   local xrange = offset_ranges[ rnd.rnd( 1, #offset_ranges ) ]
   local yrange = offset_ranges[ rnd.rnd( 1, #offset_ranges ) ]
   pos = pos + vec2.new( rnd.rnd( xrange[1], xrange[2] ), rnd.rnd( yrange[1], yrange[2] ) )
   spawn_pirate( pos )
end


function jumpout ()
   jumps_permitted = jumps_permitted - 1
   last_sys = system.cur()
   if not job_done and last_sys == missys then
      fail( fmt.f( msg[3], {sys=last_sys} ) )
   end
end


function takeoff ()
   spawn_pirate()
end


function land ()
   jumps_permitted = jumps_permitted - 1
   if job_done and planet.cur():faction() == paying_faction then
      local pay_text
      if target_killed then
         pay_text = pay_kill_text[ rnd.rnd( 1, #pay_kill_text ) ]
      else
         pay_text = pay_capture_text[ rnd.rnd( 1, #pay_capture_text ) ]
      end
      vntk.msg( _("Mission Completed"), fmt.f( pay_text, {plt=name, credits=fmt.credits(credits)} ) )
      player.pay( credits )
      paying_faction:modPlayerSingle( reputation )
      pir.reputationNormalMission( reputation )
      misn.finish( true )
   end
end


function pilot_disable ()
end


function pilot_board ()
   player.unboard()
   if can_capture then
      local t = fmt.f( subdue_text[ rnd.rnd( 1, #subdue_text ) ], {plt=name} )
      vntk.msg( _("Captured Alive"), t )
      succeed()
      target_killed = false
      target_ship:changeAI( "dummy" )
      target_ship:setHilight( false )
      target_ship:disable() -- Stop it from coming back
      hook.rm( death_hook )
   else
      local t = fmt.f( subdue_fail_text[ rnd.rnd( 1, #subdue_fail_text ) ], {plt=name} )
      vntk.msg( _("Capture Failed"), t )
      board_fail()
   end
end


function pilot_attacked( _p, attacker, dmg )
   if attacker ~= nil then
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
end


function pilot_death( _p, attacker )
   if attacker == player.pilot() or attacker:leader() == player.pilot() then
      succeed()
      target_killed = true
   else
      local top_hunter = nil
      local top_hits = 0
      local player_hits = 0
      local total_hits = 0
      for i, j in ipairs( hunters ) do
         total_hits = total_hits + hunter_hits[i]
         if j ~= nil and j:exists() then
            if j == player.pilot() or j:leader() == player.pilot() then
               player_hits = player_hits + hunter_hits[i]
            elseif hunter_hits[i] > top_hits then
               top_hunter = j
               top_hits = hunter_hits[i]
            end
         end
      end

      if top_hunter == nil or player_hits >= top_hits then
         succeed()
         target_killed = true
      elseif player_hits >= top_hits / 2 and rnd.rnd() < 0.5 then
         hailer = hook.pilot( top_hunter, "hail", "hunter_hail", top_hunter )
         credits = credits * player_hits / total_hits
         hook.pilot( top_hunter, "jump", "hunter_leave" )
         hook.pilot( top_hunter, "land", "hunter_leave" )
         hook.jumpout( "hunter_leave" )
         hook.land( "hunter_leave" )
         player.msg( "#r" .. fmt.f( msg[2], {plt=name} ) .. "#0" )
         hook.timer( 3.0, "timer_hail", top_hunter )
         misn.osdDestroy()
      else
         fail( fmt.f( msg[2], {plt=name} ) )
      end
   end
end


function pilot_jump ()
   fail( fmt.f( msg[1], {plt=name} ) )
end


function timer_hail( arg )
   if arg ~= nil and arg:exists() then
      arg:hailPlayer()
   end
end


function hunter_hail( _arg )
   hook.rm( hailer )
   hook.rm( rehailer )
   player.commClose()

   local text = share_text[ rnd.rnd( 1, #share_text ) ]
   vntk.msg( _("A Smaller Reward"), fmt.f( text, {plt=name} ) )

   player.pay( credits )
   misn.finish( true )
end


function hunter_leave ()
   misn.finish( false )
end


-- Set up the ship, credits, and reputation based on the level.
function bounty_setup ()
   local pship, credits, reputation
   if level == 1 then
      if rnd.rnd() < 0.5 then
         pship = "Hyena"
         credits = 80e3 + rnd.sigma() * 15e3
      else
         pship = "Pirate Shark"
         credits = 100e3 + rnd.sigma() * 30e3
      end
      reputation = 0.5
   elseif level == 2 then
      if rnd.rnd() < 0.5 then
         pship = "Pirate Vendetta"
      else
         pship = "Pirate Ancestor"
      end
      credits = 300e3 + rnd.sigma() * 50e3
      reputation = 1
   elseif level == 3 then
      if rnd.rnd() < 0.5 then
         pship = "Pirate Admonisher"
      else
         pship = "Pirate Phalanx"
      end
      credits = 500e3 + rnd.sigma() * 80e3
      reputation = 2
   elseif level == 4 then
      if rnd.rnd() < 0.5 then
         pship = "Pirate Starbridge"
      else
         pship = "Pirate Rhino"
      end
      credits = 700e3 + rnd.sigma() * 90e3
      reputation = 2.8
   elseif level == 5 then
      pship = "Pirate Kestrel"
      credits = 1e6 + rnd.sigma() * 100e3
      reputation = 3.5
   end
   return pship, credits, reputation
end


-- Spawn the ship at the location param.
function spawn_pirate( param )
   -- Not the time to spawn
   if job_done or system.cur() ~= missys then
      return
   end

   -- Can't jump anymore
   if jumps_permitted < 0 then
      fail( fmt.f( msg[1], {plt=name} ) )
      return
   end

   misn.osdActive( 2 )
   target_ship = pilot.add( pship, get_faction(), param )
   local aimem = target_ship:memory()
   aimem.loiter = math.huge -- Should make them loiter forever
   target_ship:rename( name )
   target_ship:setHilight( true )
   hook.pilot( target_ship, "disable", "pilot_disable" )
   hook.pilot( target_ship, "board", "pilot_board" )
   hook.pilot( target_ship, "attacked", "pilot_attacked" )
   death_hook = hook.pilot( target_ship, "death", "pilot_death" )
   pir_jump_hook = hook.pilot( target_ship, "jump", "pilot_jump" )
   pir_land_hook = hook.pilot( target_ship, "land", "pilot_jump" )

   --[[
   local pir_crew = target_ship:stats().crew
   local pl_crew = player.pilot():stats().crew
   if rnd.rnd() > (0.5 * (10 + pir_crew) / (10 + pl_crew)) then
      can_capture = true
   else
      can_capture = false
   end
   --]]
   -- Disabling and boarding is hard enough as is to randomly fail
   -- TODO potentially do a small capturing minigame here
   can_capture = true

   return target_ship
end


local _target_faction
-- Adjust pirate faction (used for "alive" bounties)
function get_faction()
   if not _target_faction then
      _target_faction = faction.dynAdd( "Pirate", "Wanted Pirate", _("Wanted Pirate"), {clear_enemies=true, clear_allies=true} )
   end
   return _target_faction
end


-- Fail to board the ship; must kill the target instead
-- (Unused in this mission; used by pirbounty_alive)
function board_fail ()
   board_failed = true
end


-- Succeed the mission, make the player head to a planet for pay
function succeed ()
   job_done = true
   misn.osdActive( 3 )
   if marker ~= nil then
      misn.markerRm( marker )
   end
   hook.rm( pir_jump_hook )
   hook.rm( pir_land_hook )
end


-- Fail the mission, showing message to the player.
function fail( message )
   if message ~= nil then
      -- Pre-colourized, do nothing.
      if message:find("#") then
         player.msg( message )
      -- Colourize in red.
      else
         player.msg( "#r" .. message .. "#0" )
      end
   end
   misn.finish( false )
end
