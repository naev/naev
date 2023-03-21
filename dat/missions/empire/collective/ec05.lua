--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Operation Black Trinity">
 <unique />
 <priority>2</priority>
 <cond>faction.playerStanding("Empire") &gt; 5 and var.peek("collective_fail") ~= true</cond>
 <done>Collective Extraction</done>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Omega Enclave</spob>
 <notes>
  <campaign>Collective</campaign>
  <tier>4</tier>
 </notes>
</mission>
--]]
--[[

   Operation Black Trinity

   Author: bobbens
      minor edits by Infiltrator

   Sixth mission in the collective mini campaign.

   Notable campaign changes:
      * Keer takes over command superseding Dimitri.
      * First real combat begins.
      * Most of the plot is unrevealed.

   Mission Objectives:
      * Attempt to arrest Zakred.
      * Kill Zakred.

   Mission Stages:
      0) Get to the Trinity
      1) Trinity Combat
      2) Trinity dead/run
      3) Ran away

-- Not too happy with the text (it chews it all up for you), but then again,
-- I'm no writer. Hopefully someone can clean it up a bit someday.

]]--
local fleet = require "fleet"
local fmt = require "format"
local emp = require "common.empire"

-- Mission constants
local misn_base, misn_base_sys = spob.getS("Omega Enclave")
local misn_target_sys = system.get("Rockbed")
local misn_flee_sys = system.get("Capricorn")

-- Non-persistent state
local drone_reinforcements, paci, trinity
local escorts = {}

local enter, trinity_flee -- Forward-declared functions

function create ()
   local missys = {misn_target_sys}
   if not misn.claim(missys) then
      misn.finish(false)
   end

   misn.setNPC( _("Dimitri?"), "unknown.webp", _("Dimitri should be around here, but you can't see him. You should probably look for him.") )
end


-- Creates the mission
function accept ()

   tk.msg( _("Bar"), _([[You enter the bar, but you can't seem to find Lt. Commander Dimitri. As you look around for him, you feel a heavy hand fall on your shoulder. It seems like two armed soldiers are here to escort you somewhere, and from the looks of their weapons, they mean business. You have no choice other then to comply.
    They start leading you away from the bar through some hallways you've never been through before. Must be all those 'Authorised Personnel Only' signs and the armed guards that didn't make them too appealing.
    Finally they toss you into what seems to be an interrogation room, simply telling you to wait.]]) )

   -- Intro text
   if not tk.yesno( _("Interrogation Room"), _([[After what seems to be a whole period, you hear the door open. You see a highly decorated woman walk in, with two soldiers standing guard at the door. She seems to be a Commodore, from the insignia on her uniform.
    "Hello, I'm Commodore Keer, I've taken over the Collective issue. I have heard about your success in the previous missions and would like to offer you more work. However, further proceedings must be kept in strict confidentiality for the interest of the Empire. You willing to go all the way with this?"]]) ) then
      misn.finish()
   end

   -- Accept the mission
   misn.accept()

   -- Mission data
   mem.misn_stage = 0
   mem.misn_marker = misn.markerAdd( misn_target_sys, "high" )

   -- Mission details
   misn.setTitle(_("Operation Black Trinity"))
   misn.setReward( emp.rewards.ec05 )
   misn.setDesc( fmt.f(_("Arrest the ESS Trinity in {sys}"), {sys=misn_target_sys} ))
   misn.osdCreate(_("Operation Black Trinity"), {
      fmt.f(_("Fly to the {sys} system"), {sys=misn_target_sys}),
      _("Apprehend or kill Zakred"),
      fmt.f(_("Report back to {pnt}"), {pnt=misn_base}),
   })

   tk.msg( _("Interrogation Room"), fmt.f(_([[You accept and she dismisses both of the soldiers, who proceed to wait outside.
    "We've been following Lt. Commander Dimitri's progress since he started at {pnt}. The datapad you brought back has confirmed what we have suspected. We have an undercover Collective agent somewhere in the military who's been feeding ex-Commodore Welsh data. You don't understand, right? Let me explain."]]), {pnt=misn_base} ) )
   tk.msg( _("Operation Black Trinity"), fmt.f(_([["The Collective was actually a project for the Empire. They were supposed to be the ultimate weapon in flexibility and offense. Commodore Welsh was in charge of the secret science facility on {pnt}. Shortly after the Incident, we stopped hearing from them. We sent a recon and were met with hostile Collective drones. It seems like the project had been a success, but the traitor, Welsh, went rogue. Under normal circumstances we would have easily crushed the Collective, but after the Incident these are hardly normal circumstances."
    She goes on. "Things have gotten out of hand. We have had chances to crush Welsh, but he always seems to evade us and strike where we are weakest. We always knew there must have been another traitor in our midst, but with the datapad information we now know who he is."]]), {pnt=_("Eiroik")}))
   emp.addCollectiveLog( _([[Commodore Keer has taken over the Collective issue and explained more about the Collective. "The Collective was actually a project for the Empire. They were supposed to be the ultimate weapon in flexibility and offense. Commodore Welsh was in charge of the secret science facility on Eiroik. Shortly after the Incident, we stopped hearing from them. We sent a recon and were met with hostile Collective drones. It seems like the project had been a success, but the traitor Welsh went rogue. Under normal circumstances we would have easily crushed the Collective, but after the Incident these are hardly normal circumstances."]]) )
   tk.msg( _("Mission Accomplished"), fmt.f(_([[She now clears her throat. "This operation has been dubbed 'Operation Black Trinity'. We have reason to believe that the ESS Trinity has been operating with the traitor Welsh. The ESS Trinity is commanded by Captain Zakred. You will form part of an assault team with the primary objective of arresting Zakred. As a last resort, you are authorized to kill Zakred. He must not escape.
    "We'll be sending you with a small force. We expect you to hang back, but, if any trouble arises, take the ESS Trinity down. Zakred is currently on manoeuvre exercises in {sys}. You will have to find him there. The other ships will follow your lead to {sys}. Good luck."]]), {sys=misn_target_sys} ) )

   -- Escorts
   hook.jumpout("jumpout")
   hook.jumpin("jumpin")
   hook.takeoff("takeoff")
   hook.land("land")
end


function jumpout ()
   mem.last_sys = system.cur()
end


function jumpin ()
   enter( mem.last_sys )
end

function takeoff ()
   enter( nil )
end


-- Handles jumping to target system
function enter ( from_sys )
   -- Only done for stage 1
   if mem.misn_stage == 0 then
      local sys = system.cur()

      -- Create some havoc
      if sys == misn_target_sys then
         -- Escorts enter a while back
         mem.enter_vect = player.pos()
         if from_sys == nil then
            add_escorts( true )
         else -- Just jumped
            hook.timer( rnd.uniform(2.0, 5.0), "add_escorts" )
         end

         -- Disable spawning and clear pilots -> makes it more epic
         pilot.clear()
         pilot.toggleSpawn(false)

         mem.misn_stage = 1
         misn.osdActive(2)

         -- Position trinity on the other side of the player
         trinity = pilot.add( "Empire Hawking", "Empire", vec2.new(-5000, 1500), _("ESS Trinity"), {ai="noidle"} )
         trinity:setVisplayer()
         trinity:setHilight(true)
         trinity:setFaction("Empire") -- Starts out non-hostile
         trinity:setNoDisable(true)
         hook.pilot( trinity, "death", "trinity_kill" )
         hook.pilot( trinity, "jump", "trinity_jump" )

         mem.final_fight = 0
         hook.timer(rnd.uniform(6.0, 8.0) , "final_talk") -- Escorts should be in system by now
      end

   -- Player ran away from combat - big disgrace.
   elseif mem.misn_stage == 1 then

      mem.misn_stage = 3
      player.msg( _("Mission Failure: Return to base.") )
      misn.setDesc( fmt.f(_("Return to base at {pnt} in {sys}"),
            {pnt=misn_base, sys=misn_base_sys} ))
      misn.markerMove( mem.misn_marker, misn_base )
   end
end


-- Little talk when ESS Trinity is encountered.
function final_talk ()
   -- Empire talks about arresting
   local talker
   if mem.final_fight == 0 then
      talker = paci
      talker:broadcast( _("ESS Trinity: Please turn off your engines and prepare to be boarded.") )

      mem.final_fight = 1
      hook.timer(rnd.uniform( 3.0, 4.0 ), "final_talk")
   elseif mem.final_fight == 1 then
      talker = trinity
      talker:broadcast( _("You will never take me alive!") )

      mem.final_fight = 2
      hook.timer(rnd.uniform( 3.0, 4.0 ), "final_talk")
   elseif mem.final_fight == 2 then
      -- Talk
      talker = paci
      talker:broadcast( _("Very well then. All units engage ESS Trinity.") )

      -- ESS Trinity becomes collective now.
      trinity:setFaction("Collective")
      trinity:setHostile()

      mem.final_fight = 3
      hook.timer(rnd.uniform( 4.0, 5.0 ), "final_talk")
   elseif mem.final_fight == 3 then
      mem.tri_flee  = false
      hook.timer( 3.0, "trinity_check" )
      mem.tri_checked = 0
      for i, j in ipairs( escorts ) do
         if j:exists() then
            j:control(false)
         end
      end
   end
end


function trinity_check ()
   if mem.tri_flee then
      return
   end
   mem.tri_checked = mem.tri_checked + 1

   if mem.tri_checked == 3 then
      trinity:broadcast( _("It is too late! The plan is being put into motion!") )
   elseif mem.tri_checked == 6 then
      trinity:broadcast( _("You have no idea who you're messing with!") )
   end

   local a = trinity:health()
   if a < 100 or mem.tri_checked > 100 then
      trinity_flee()
   else
      hook.timer( 3.0, "trinity_check" )
   end
end


-- Trinity runs away
function trinity_flee ()
   mem.tri_flee = true
   trinity:control()
   trinity:hyperspace( misn_flee_sys )
   trinity:broadcast( _("My drones will make mincemeat of you!") )
   player.msg( _("Incoming drones from hyperspace detected!") )
   hook.timer( rnd.uniform( 3.0, 5.0 ), "call_drones_jump" )
end


-- Jump in support
function call_drones_jump ()
   local sml_swarm = { "Drone", "Drone", "Drone", "Heavy Drone" }
   drone_reinforcements = fleet.add( 1, sml_swarm, "Collective", misn_flee_sys, _("Collective Drone") )
   drone_reinforcements[4]:rename(_("Collective Heavy Drone"))
   mem.drone_controlled = true
   local tp = trinity:pos()
   for k,v in ipairs(drone_reinforcements) do
      v:setHostile()
      v:setNoDisable(true)
      v:control()
      v:moveto( tp )
      hook.pilot( v, "attacked", "drone_attacked" )
      hook.pilot( v, "idle",     "drone_attacked" )
   end
end


-- Support drones attacked
function drone_attacked ()
   if mem.drone_controlled then
      mem.drone_controlled = false
      for k,v in ipairs(drone_reinforcements) do
         if v:exists() then
            v:control(false)
         end
      end
   end
end


-- Adds escorts
function add_escorts( landed )
   local param
   if landed then
      param = mem.enter_vect
   else
      param = mem.last_sys
   end

   paci = pilot.add( "Empire Pacifier", "Empire", param, nil, {ai="escort_player"} )
   escorts[#escorts + 1] = paci
   paci:setFriendly()
   if trinity ~= nil then
      paci:control()
      paci:moveto( trinity:pos() )
   end
   for i=1, 6 do
      local lance = pilot.add( "Empire Lancelot", "Empire", param, nil, {ai="escort_player"} )
      escorts[#escorts + 1] = lance
      lance:setFriendly()
      if trinity ~= nil then
         lance:control()
         lance:moveto( trinity:pos() )
      end
   end
end


-- Handles arrival back to base
function land ()
   local pnt = spob.cur()
   local credits = emp.rewards.ec05

   -- Just landing
   if (mem.misn_stage == 2 or mem.misn_stage == 3) and pnt == misn_base then

      if mem.trinity_alive or mem.misn_stage == 3 then
         -- Failure to kill
         tk.msg( _("Mission Failure"), _([[You see Commodore Keer with a dozen soldiers waiting for you outside the landing pad.
    "You weren't supposed to let the Trinity get away! Now we have no cards to play. We must wait for the Collective response or new information before being able to continue. We'll notify you if we have something you can do for us, but for now we just wait."]]) )
         var.push("trinity", true)
         credits = credits / 2
         emp.addCollectiveLog( _([[You failed to destroy the ESS Trinity, putting a hitch in the Empire's plans. You should meet back with Commodore Keer at the bar on Omega Enclave; she said that they would notify you if they have something more that you can do.]]) )
      else
         -- Successfully killed
         tk.msg( _("Mission Accomplished"), fmt.f(_([[You see Commodore Keer with a dozen soldiers waiting for you outside the landing pad.
    "Congratulations on the success, {player}. We never really expected to take Zakred alive. Good riddance. The next step is to begin an all-out attack on Collective territory. Meet up in the bar when you're ready. We'll need all available pilots."]]), {player=player.name()}) )
         var.push("trinity", false)
         emp.addCollectiveLog( _([[You successfully killed Zakred (a Collective spy) and destroyed the ESS Trinity. Commodore Keer told you to meet her again at the bar on Omega Enclave for an all-out attack on Collective territory.]]) )
      end

      -- Rewards
      player.pay(credits)
      faction.modPlayerSingle("Empire",5)

      misn.finish(true)
   end
end


-- Trinity hooks
function trinity_kill () -- Got killed
   player.msg( _("Mission Success: Return to base.") )
   mem.misn_stage = 2
   misn.osdActive(3)
   mem.trinity_alive = false
   misn.setDesc( fmt.f(_("Return to base at {pnt} in {sys}"), {pnt=misn_base, sys=misn_base_sys} ))
   misn.markerMove( mem.misn_marker, misn_base )
end


function trinity_jump () -- Got away
   player.msg( _("Mission Failure: Return to base.") )
   mem.misn_stage = 2
   misn.osdActive(3)
   mem.trinity_alive = true
   misn.setDesc( fmt.f(_("Return to base at {pnt} in {sys}"), {pnt=misn_base, sys=misn_base_sys} ))
   misn.markerMove( mem.misn_marker, misn_base )
end
