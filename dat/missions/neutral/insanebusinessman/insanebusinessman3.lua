--[[Insane Businessman Part 2]]--



-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then -- Spanish version of the texts would follow
elseif lang == "de" then -- German version of the texts would follow 
else -- default English text

--[[This section is used to define lots of variables holding all the various texts.]]--

npc_name = "A Businessman"
bar_desc = "A disheveled but familiar looking businessman."
title = "Insane Businessman Part 3"
pre_accept = {}

pre_accept[1] = [["Thank you for speaking to me again,"says Crumb. "I have a really important mission for you."]] 

pre_accept[2] = [[With a stern look on his face, Crumb begins to speak, "Another pilot has achieved what you could not, %s, he has gotten me information about a plot to kill me. That's ok. Your job now is to rendezvous with this pilot and his two associates in the uninhabited %s system. From there all four of you will travel to %s and await further instructions. I guarantee, once all this is over, you will be very happy...eh, financially..."]]

pre_accept[3] = [[You think to yourself that you usually work alone. Do you take the mission anyway?]]

decline = [["Ah, well some other time maybe..."]]

misn_desc = "Fly to the %s system and find the rendezvous. Once you find him, fly to %s and await further instructions."
reward_desc = "%s credits on completion."
post_accept = {}
post_accept[1] = [["Rendezvous with your contact in %s then fly to the %s and await further instructions."]]

misn_accomplished = [[You land back in %s but find Crumb a difficult person to locate. He must have fled knowing that you defeated his mercenaries. However he couldn't have gone far...]]

rendezvous_title = [[Rendezvous]]
rendezvous_msg = [["%s! We've been waiting for you." says a voice over the comm. 
"Great. Let's get going."
"In a minute. There's been a slight change of plans. Mr Crumb has decided your services are no longer necessary. Sorry about this, just business"
click!
]]

-- OSD

OSDtitle1 = "Find Crumb's Rendezvous"
OSDdesc11 = "Go to the %s system."
OSDdesc12 = "Find rendezvous."
OSDdesc13 = "Go to the %s system and await orders."

OSDtitle2 = "Confront Crumb"
OSDdesc21 = "Return to the %s system and confront Crumb."

OSDtable1 = {}
OSDtable2 = {}

end


-- Messages
msg = {}
msg[1] = "MISSION FAILED!!!! You killed your Rendezvous."

--[[

After the texts follow the functions.
There are bascially two types, those defined inside this file and the API functions.]]--

function create ()
   -- This will get called when the conditions in mission.xml are met (or when the mission is initiated from another script).
   -- It is used to set up mission variables.

   -- Get the planet and system at which we currently are.
   startworld, startworld_sys = planet.cur()

   -- Set our target system and planet.
   targetworld_sys = system.get("Haven")
   fake_targetworld_sys = system.get("Alteris")

   -- IMPORTANT: system claiming
   -- Missions and events may "claim" one or more systems for prioritized use. When a mission has claimed a system, it acquires the "right" to temporarily modify that system.
   -- For example, all the pilots may be cleared out, or the spawn rates may be changed. Obviously, only one mission may do this at a time, or serious conflicts ensue.
   -- Therefore, you have to make your mission claim any systems you want to get privileged rights on.
   -- When a mission tries to claim a system that is already claimed, the mission MUST terminate.
   -- If you do not need to claim any systems, please make a comment at the beginning of the create() function that states so.
   if not misn.claim ( {targetworld_sys} ) then
      abort() -- Note: this assumes you have an abort() function in your script. You may also just use misn.finish() here.
   end

   -- Set a reward. This is just a useful variable, nothing special.
   reward = 10000
   dead_pilots = 0
   -- Set stuff up for the bar.
   -- Give our NPC a name and a portrait.
   misn.setNPC( npc_name, "neutral/unique/shifty_merchant" )
   -- Describe what the user should see when they click on the NPC in the bar.
   misn.setDesc( bar_desc )
end


function accept ()
  
   -- Some background mission text
   tk.msg( title, pre_accept[1] )
   tk.msg( title, string.format( pre_accept[2], player.name(), targetworld_sys:name(), fake_targetworld_sys:name() ) )

   -- Show a yes/no dialog with a title and a text.
   -- If the answer is no the following block will be executed.
   -- In this case, misn.finish(), it ends the mission without changing its status.

   if not tk.yesno( title, pre_accept[3] ) then

      tk.msg( title, decline )

      misn.finish()
   end

   -- Set up mission information for the onboard computer and OSD.
   -- The OSD Title takes up to 11 signs.
   misn.setTitle( title )

   -- Reward is only visible in the onboard computer.
   misn.setReward( string.format( reward_desc, reward ) )

   -- Description is visible in OSD and the onboard computer, it shouldn't be too long either.
   misn.setDesc( string.format( misn_desc, targetworld_sys:name(), fake_targetworld_sys:name() ) )

   -- Set marker to a system, visible in any mission computer and the onboard computer.
   landmarker = misn.markerAdd( targetworld_sys, "low")


   -- Add mission
   -- At this point the mission gets added to the players active missions.
   misn.accept()

   -- Create a windows that show up after the player has accepted.
   -- Useful to explain further details.
   tk.msg( title, string.format( post_accept[1], targetworld_sys:name(), fake_targetworld_sys:name() ) )

   -- Set OSD after accpeting the mission
   OSDtable1[1] = OSDdesc11:format( targetworld_sys:name() )
   OSDtable1[2] = OSDdesc12
   OSDtable1[3] = OSDdesc13:format( fake_targetworld_sys:name() )
   misn.osdCreate( OSDtitle1, OSDtable1 )
   misn.osdActive(1)

   

-- Uncomment  this line to reveal the planet Samson is in.
--   tk.msg( title, targetworld:name() )

   -- Set up hooks.
   -- These will be called when a certain situation occurs ingame..
   hook.land("land")
   hook.jumpin("jumpin")
   hook.jumpout("jumpout")
end

   -- The function specified in the above hook.
function land ()
   if planet.cur() == startworld and victorious then
       
      -- Pop up a window that tells the player they finished the mission and got their reward.
      tk.msg( title,  string.format(misn_accomplished, startworld:name() ) )

      -- Mark the mission as successfully finished.
      misn.finish(true)   
   end 
 
end

--If you manage to escape you still win
function jumpout()
   if system.cur() == targetworld_sys and hailed then
      completeOSDSet()
      victorious = true   
   end

   if system.cur() == targetworld_sys and not hailed then
       misn.osdActive(1)
   end

end

-- Moves to the next point upon jumping into target system
function jumpin ()
   if system.cur() == targetworld_sys and not hailed then
      misn.osdActive(2)
      spawn_pilots()
   end
end

-- Spawn pilot function
function spawn_pilots()
   
   from_system = system.get("Daled")  

   pilot1 = pilot.addRaw( "Admonisher", "mercenary" , from_system , "Dummy")
   pilot1[1]:rename("Rendezvous")
   pilot1[1]:setHilight( true )
   pilot1[1]:addOutfit("Laser Turret MK1", 2)
   pilot1[1]:addOutfit("Laser Turret MK2", 1)
   pilot1[1]:addOutfit("Gauss Gun", 2)
   pilot1[1]:control()
   pilot1[1]:follow(player.pilot())
   hook.pilot( pilot1[1], "death", "pilot_death" )
   hook.pilot( pilot1[1], "attacked", "pilot_attacked")

   pilot2 = pilot.addRaw( "Admonisher", "mercenary" , from_system , "Dummy")
   pilot2[1]:rename("Rendezvous")
   pilot2[1]:setHilight( true )
   pilot2[1]:addOutfit("Laser Turret MK1", 2)
   pilot2[1]:addOutfit("Laser Turret MK2", 1)
   pilot2[1]:addOutfit("Gauss Gun", 2)
   pilot2[1]:control()
   pilot2[1]:follow(player.pilot())
   hook.pilot( pilot2[1], "death", "pilot_death" ) 
   hook.pilot( pilot2[1], "attacked", "pilot_attacked")

   pilot3 = pilot.addRaw( "Admonisher", "mercenary" , from_system , "Dummy")
   pilot3[1]:rename("Rendezvous")
   pilot3[1]:setHilight( true )
   pilot3[1]:addOutfit("Laser Turret MK1", 2)
   pilot3[1]:addOutfit("Laser Turret MK2", 1)
   pilot3[1]:addOutfit("Gauss Gun", 2)
   pilot3[1]:control()
   pilot3[1]:follow(player.pilot())
   hook.pilot( pilot3[1], "death", "pilot_death" )
   hook.pilot( pilot3[1], "attacked", "pilot_attacked")

   pilot1[1]:hailPlayer()
   hailing = hook.pilot(pilot1[1], "hail","hailme")

end

--If you attack the pilots before answering the hail, they will defend themselves
function pilot_attacked()
   if pilot1[1]:exists() then
      pilot1[1]:control()
      pilot1[1]:setHostile(true)
      pilot1[1]:attack(player.pilot())
      pilot1[1]:hailPlayer(false)
   end
  
   if pilot2[1]:exists() then
      pilot2[1]:control()
      pilot2[1]:setHostile(true)
      pilot2[1]:attack(player.pilot())
   end
   
   if pilot3[1]:exists() then
      pilot3[1]:control()
      pilot3[1]:setHostile(true)
      pilot3[1]:attack(player.pilot())
   end

end

function hailme()
    player.commClose()
    tk.msg(rendezvous_title, string.format(rendezvous_msg, player.name()))
    hailed = true
    pilot_attacked()
end

--Executes if you kill him
function pilot_death()
   if not hailed then
      fail(1)
   end
   
   if dead_pilots < 3 then
      dead_pilots = dead_pilots + 1
   end

   if hailed and dead_pilots == 3 then
      completeOSDSet()
      victorious = true
   end 
end

--Mission fail function
function fail(param)
   player.msg( msg[param] )
   misn.finish( false )
end

-- This will be called when the player aborts the mission in the onboard computer.
function abort ()
   -- Mark mission as unsuccessfully finished. It won't show up again if this mission is marked unique in mission.xml.
   misn.finish( false )
end

function completeOSDSet()
   misn.osdDestroy ()
   OSDtable2[1] = OSDdesc21:format( targetworld_sys:name() )
   misn.osdCreate( OSDtitle2, OSDtable2 )
   misn.markerMove(landmarker, startworld_sys)
end
