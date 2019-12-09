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

include "numstring.lua"

bar_desc = _("Dimitri should be around here, but you can't see him. You should probably look for him.")
misn_title = _("Operation Black Trinity")
misn_reward = _("%s credits")
misn_desc = {}
misn_desc[1] = _("Arrest the ESS Trinity in %s.")
misn_desc[2] = _("Return to base at %s in %s.")
title = {}
title[1] = _("Bar")
title[2] = _("Interrogation Room")
title[3] = _("Operation Black Trinity")
title[4] = _("Mission Accomplished")
title[5] = _("Mission Failure")
text = {}
text[1] = _([[You enter the bar, but you can't seem to find Lt. Commander Dimitri. As you look around for him, you feel a heavy hand fall on your shoulder. It seems like two armed soldiers are here to escort you somewhere, and from the looks of their weapons, they mean business. You have no choice other then to comply.
    They start leading you away from the bar through some hallways you've never been through before. Must be all those 'Authorised Personnel Only' signs and the armed guards that didn't make them too appealing.
    Finally they toss you into what seems to be an interrogation room, simply telling you to wait.]])
text[2] = _([[After what seems to be the whole period, you hear the door open. You see a highly decorated woman walk in, with two soldiers standing guard at the door. She seems to be a Commodore, from the insignia on her uniform.
    "Hello, I'm Commodore Keer, I've taken over the Collective issue. I have heard about your success in the previous missions and would like to offer you more work. However, further proceedings must be kept in strict confidentiality for the interest of the Empire. You willing to go all the way with this?"]])
text[3] = _([[You accept and she dismisses both of the soldiers, who proceed to wait outside.
    "We've been following Lt. Commander Dimitri's progress since he started at %s. The datapad you brought back has confirmed what we have suspected. We have a undercover Collective agent somewhere in the military who's been feeding ex-Commodore Welsh data. You don't understand, right? Let me explain."]])
text[4] = _([["The Collective was actually a project for the Empire. They were supposed to be the ultimate weapon in flexibility and offense. Commodore Welsh was in charge of the secret science facility on %s. Shortly after the Incident, we stopped hearing from them. We sent a recon and were met with hostile Collective drones. It seems like the project had been a success, but the traitor Welsh went rogue. Under normal circumstances we would have easily crushed the Collective, but after the Incident these are hardly normal circumstances."
    She goes on. "Things have gotten out of hand. We have had chances to crush Welsh, but he always seems to evade us and strike where we were weakest. We always knew there must have been another traitor in our midst, but with the datapad information we now know who he is."]])
text[5] = _([[She now clears her throat. "This operation has been dubbed 'Operation Black Trinity'. We have reason to believe that the ESS Trinity has been operating with the traitor Welsh. The ESS Trinity is commanded by Captain Zakred. You will form part of an assault team with the primary objective of arresting Zakred. If all goes to worse, you are ordered to kill Zakred. He must not escape."
    "We'll be sending you with a small force. You just stick around and if any trouble arises, take the ESS Trinity down. Zakred is currently on manoeuvre exercises in %s. You will have to find him there. The other ships will follow your lead to %s. Good luck."]])
text[6] = _([[You see Commodore Keer with a dozen soldiers waiting for you outside the landing pad.
    "Congratulations on the success, %s. We never really expected to take Zakred alive. Good riddance. The next step is to begin an all out attack on Collective territory. Meet up in the bar when you're ready. We'll need all available pilots."]])
text[7] = _([[You see Commodore Keer with a dozen soldiers waiting for you outside the landing pad.
    "You weren't supposed to let the Trinity get away! Now we have no cards to play. We must wait for the Collective response or new information before being able to continue. We'll notify you if we have something you can do for us, but for now we just wait."]])

-- Conversation between pilots
talk = {}
talk[1] = _("ESS Trinity: Please turn off your engines and prepare to be boarded.")
talk[2] = _("You will never take me alive!")
talk[3] = _("Very well then. All units engage ESS Trinity.")
talk[4] = _("The Trinity has launched Collective drones!")
talk[5] = _("Mission Success: Return to base.")
talk[6] = _("Mission Failure: Return to base.")
talk[7] = _("Incoming drones from hyperspace detected!")

osd_msg = {}
osd_msg[1] = _("Fly to the %s system")
osd_msg[2] = _("Apprehend or kill Zakred")
osd_msg[3] = _("Report back to %s")
osd_msg["__save"] = true

taunts = {}
taunts[1] = _("It is too late! The plan is being put into movement!")
taunts[2] = _("You have no idea who you're messing with!")
taunts[3] = _("My drones will make mincemeat of you!")


function create ()
   missys = {system.get("Rockbed")}
   if not misn.claim(missys) then
      misn.finish(false)
   end
 
   misn.setNPC( _("Dimitri?"), "unknown" )
   misn.setDesc( bar_desc )
   credits = 2000000
end


-- Creates the mission
function accept ()

   tk.msg( title[1], text[1] )

   -- Intro text
   if not tk.yesno( title[2], text[2] ) then
      misn.finish()
   end

   -- Accept the mission
   misn.accept()

   -- Mission data
   misn_stage = 0
   misn_base, misn_base_sys = planet.get("Omega Station")
   misn_target_sys = system.get("Rockbed")
   misn_flee_sys = system.get("Capricorn")
   misn_marker = misn.markerAdd( misn_target_sys, "high" )

   -- Mission details
   misn.setTitle(misn_title)
   misn.setReward( misn_reward:format( numstring( credits ) ) )
   misn.setDesc( string.format(misn_desc[1], misn_target_sys:name() ))
   osd_msg[1] = osd_msg[1]:format(misn_target_sys:name())
   osd_msg[3] = osd_msg[3]:format(misn_base:name())
   misn.osdCreate(misn_title, osd_msg)
   
   tk.msg( title[2], string.format(text[3], misn_base:name() ) )
   tk.msg( title[3], string.format(text[4], "Eiroik"))
   tk.msg( title[4], string.format(text[5], misn_target_sys:name(), misn_target_sys:name() ) )

   -- Escorts
   escorts = {}

   hook.jumpout("jumpout")
   hook.jumpin("jumpin")
   hook.takeoff("takeoff")
   hook.land("land")
end


function jumpout ()
   last_sys = system.cur()
end


function jumpin ()
   enter( last_sys )
end

function takeoff ()
   enter( nil )
end


-- Handles jumping to target system
function enter ( from_sys )
   -- Only done for stage 1
   if misn_stage == 0 then
      local sys = system.cur()

      -- Create some havoc
      if sys == misn_target_sys then
         -- Escorts enter a while back
         enter_vect = player.pos()
         if from_sys == nil then
            add_escorts( true )
         else -- Just jumped
            hook.timer( rnd.int(2000, 5000), "add_escorts" )
         end

         -- Disable spawning and clear pilots -> makes it more epic
         pilot.clear()
         pilot.toggleSpawn(false)

         misn_stage = 1
         misn.osdActive(2)

         -- Position trinity on the other side of the player
         trinity = pilot.add("Trinity", "noidle", vec2.new(-5000, 1500))[1]
         trinity:setVisplayer()
         trinity:setHilight(true)
         trinity:setFaction("Empire") -- Starts out non-hostile
         trinity:setNodisable(true)
         hook.pilot( trinity, "death", "trinity_kill" )
         hook.pilot( trinity, "jump", "trinity_jump" )

         final_fight = 0
         hook.timer(rnd.int(6000, 8000) , "final_talk") -- Escorts should be in system by now
      end

   -- Player ran away from combat - big disgrace.
   elseif misn_stage == 1 then

      misn_stage = 3
      player.msg( talk[6] )
      misn.setDesc( string.format(misn_desc[2],
            misn_base:name(), misn_base_sys:name() ))
      misn.markerMove( misn_marker, misn_base_sys )
   end
end


-- Little talk when ESS Trinity is encountered.
function final_talk ()
   -- Empire talks about arresting
   if final_fight == 0 then
      talker = paci
      talker:broadcast( talk[1] )

      final_fight = 1
      hook.timer(rnd.int( 3000, 4000 ), "final_talk")
   elseif final_fight == 1 then
      talker = trinity
      talker:broadcast( talk[2] )

      final_fight = 2
      hook.timer(rnd.int( 3000, 4000 ), "final_talk")
   elseif final_fight == 2 then
      -- Talk
      talker = paci
      talker:broadcast( talk[3] )

      -- ESS Trinity becomes collective now.
      trinity:setFaction("Collective")
      trinity:setHostile()

      final_fight = 3
      hook.timer(rnd.int( 4000, 5000 ), "final_talk")
   elseif final_fight == 3 then
      player.msg( talk[4] )
      tri_flee  = false
      hook.timer(rnd.int( 3000, 5000 ) , "call_drones")
      hook.timer( 3000, "trinity_check" )
      tri_checked = 0
      for i, j in ipairs( escorts ) do
         if j:exists() then
            j:control(false)
         end
      end
   end
end


function trinity_check ()
   if tri_flee then
      return
   end
   tri_checked = tri_checked + 1

   if tri_checked == 3 then
      trinity:broadcast( taunts[1] )
   elseif tri_checked == 6 then
      trinity:broadcast( taunts[2] )
   end

   local a,s = trinity:health()
   if s < 70 or tri_checked > 10 then
      trinity_flee()
   else
      hook.timer( 3000, "trinity_check" )
   end
end


-- Trinity runs away
function trinity_flee ()
   tri_flee = true
   trinity:control()
   trinity:hyperspace( misn_flee_sys, true )
   trinity:broadcast( taunts[3] )
   player.msg( talk[7] )
   hook.timer( rnd.int( 3000, 5000 ), "call_drones_jump" )
end

-- Calls help for the ESS Trinity.
function call_drones ()
   local pilots = pilot.add("Collective Sml Swarm", nil, trinity:pos())
   num_drone = #pilots
   for k,v in ipairs(pilots) do
      v:setHostile()
      v:setNodisable(true)
      hook.pilot( v, "death", "drone_dead" )
   end
end

-- Jump in support
function call_drones_jump ()
   drone_reinforcements = pilot.add("Collective Sml Swarm", nil, misn_flee_sys)
   drone_controlled = true
   local tp = trinity:pos()
   for k,v in ipairs(drone_reinforcements) do
      v:setHostile()
      v:setNodisable(true)
      v:control()
      v:goto( tp )
      hook.pilot( v, "attacked", "drone_attacked" )
      hook.pilot( v, "idle",     "drone_attacked" )
   end
end

-- Support drones attacked
function drone_attacked ()
   if drone_controlled then
      drone_controlled = false
      for k,v in ipairs(drone_reinforcements) do
         if v:exists() then
            v:control(false)
         end
      end
   end
end
-- Drone killed
function drone_dead ()
   num_drone = num_drone - 1
   if num_drone < 2 and not tri_flee then
      trinity_flee()
   end
end


-- Adds escorts
function add_escorts( landed )
   local param
   if landed then
      param = enter_vect
   else
      param = last_sys
   end
   
   if escorts == nil then escorts = {} end
   paci = pilot.add("Empire Pacifier", "escort_player", param)[1]
   escorts[#escorts + 1] = paci
   paci:setFriendly()
   if trinity ~= nil then
      paci:control()
      paci:goto( trinity:pos() )
   end
   for i=1, 6 do
      local lance = pilot.add("Empire Lancelot", "escort_player", param)[1]
      escorts[#escorts + 1] = lance
      lance:setFriendly()
      if trinity ~= nil then
         lance:control()
         lance:goto( trinity:pos() )
      end
   end
end


-- Handles arrival back to base
function land ()
   local pnt = planet.cur()

   -- Just landing
   if (misn_stage == 2 or misn_stage == 3) and pnt == misn_base then

      if trinity_alive or misn_stage == 3 then
         -- Failure to kill
         tk.msg( title[5], text[7] )
         var.push("trinity", true)
         credits = credits / 2
      else
         -- Successfully killed
         tk.msg( title[4], string.format(text[6], player.name()) )
         var.push("trinity", false)
      end

      -- Rewards
      player.pay(credits)
      faction.modPlayerSingle("Empire",5)

      misn.finish(true)
   end
end


-- Trinity hooks
function trinity_kill () -- Got killed
   player.msg( talk[5] )
   misn_stage = 2
   misn.osdActive(3)
   trinity_alive = false
   misn.setDesc( string.format(misn_desc[2], misn_base:name(), misn_base_sys:name() ))
   misn.markerMove( misn_marker, misn_base_sys )
end
function trinity_jump () -- Got away
   player.msg( talk[6] )
   misn_stage = 2
   misn.osdActive(3)
   trinity_alive = true
   misn.setDesc( string.format(misn_desc[2], misn_base:name(), misn_base_sys:name() ))
   misn.markerMove( misn_marker, misn_base_sys )
end
