--[[Insane Businessman Part 2]]--



-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then -- Spanish version of the texts would follow
elseif lang == "de" then -- German version of the texts would follow 
else -- default English text

--[[This section is used to define lots of variables holding all the various texts.]]--

npc_name = "A Businessman"
bar_desc = "A disheveled looking businessman"
title = "Insane Businessman Part 2"
pre_accept = {}

pre_accept[1] = [["Glad you could join me again,"says Crumb. "I think I know who's been feeding me the misinformation."]] 

pre_accept[2] = [["A former associate of mine, Phillip Samson, has been hanging around the %s system. Find him and get him to talk. I know he's involved in the plots against me."]]

pre_accept[3] = [[If there even are plots against him, you can't help but think. Do you accept the mission?]]

decline = [["Ah, well some other time maybe..."]]

misn_desc = "Fly to the %s system and find Samson. Once you find him, interrogate him and get him to talk."
reward_desc = "%s credits on completion."
post_accept = {}
post_accept[1] = [["Find Samson in the %s system and get him to talk. I don't care how, just get me some information."]]

misn_investigate = [[You check out all the local hangouts that Crumb told you Samson patrons. Turning up nothing you think you're completely out of luck when at last fortune shines upon you. A nondescript bartender tell you that he knows Samson. He also tells you Samson knows you're looking for him as someone has tipped him off. He finally says Samson was headed to the spaceport and was planning on leaving the planet and system.]]

misn_accomplished = [[When you tell Crumb what Samson has told you Crumb is enraged. He takes a deep breath and immediately calms down. "Rubbish! Someone is trying to kill me and I'm going to find out who it is! Well, here's your credits."

Oh so now someone is trying to kill him, you can't help but think. Funny how things are progressing. 

"I need you to do one more mission for me. Meet me at the bar," Crumb says, before storming off. 
]]

board_title = [[Samson's Ship]]
board_msg = [[You board Samson's ship. "Please don't hurt me!" Samson yells. 
"I'm not going to hurt you," you reply, "I just want information." 
"What's there to say. Crumb is crazy. We used to be associates. We ran a business togeather. Selling and buying property. It's just that after..."
"After what?," you inquire.
"There was an electrical accident in one of his hotels. Killed like three people.  The Empire went after him pretty hard for negligence. Ever since then he's progressively gotten more paranoid, alienating his friends and family. We used to be pretty close you know, but now...now, I think he's trying to kill me. Well, you can understand why I ran."
Strange. Crumb thinks Samson is trying to ruin him, but Samson thinks Crumb sent me to kill him! Better return to Crumb and report this.
]]

-- OSD

OSDtitle1 = "Find Samson and get him to talk."
OSDdesc1 = "Go to the %s system."
OSDdesc2 = "Find Samson."
OSDdesc3 = "Catch Samson before he gets away."
OSDdesc4 = "Return to %s and report to Crumb."

OSDtitle2 = "Catch Samson before he gets away."
OSDtable1 = {}
OSDtable2 = {}

OSDtitle3 = "Report to Crumb."
OSDtable3 = {}

-- Messages

msg    = {}
msg[1] = "MISSION FAILURE! Samson got away."
msg[2] = "MISSION FAILURE! You killed Samson."

end



--[[

After the texts follow the functions.
There are bascially two types, those defined inside this file and the API functions.]]--

function create ()
   -- This will get called when the conditions in mission.xml are met (or when the mission is initiated from another script).
   -- It is used to set up mission variables.

   -- Get the planet and system at which we currently are.
   startworld, startworld_sys = planet.cur()

   -- Set our target system and planet. Picks a random planet in Delta Pavonis
   targetworld_sys = system.get("Delta Pavonis")
   targetworlds = {}
   i = 0
   for key, planet in ipairs( targetworld_sys:planets() ) do
      if planet:canLand() then
         i = i + 1
         targetworlds[i] = planet  
      end 
   end   

   targetworld_number = rnd.rnd(1,i)
   targetworld = targetworlds[targetworld_number]


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

   -- Set stuff up for the bar.
   -- Give our NPC a name and a portrait.
   misn.setNPC( npc_name, "neutral/unique/shifty_merchant" )
   -- Describe what the user should see when they click on the NPC in the bar.
   misn.setDesc( bar_desc )
end


function accept ()
  
   -- Some background mission text
   tk.msg( title, pre_accept[1] )
   tk.msg( title, string.format( pre_accept[2], targetworld_sys:name() ) )

   -- Show a yes/no dialog with a title and a text.
   -- If the answer is no the following block will be executed.
   -- In this case, misn.finish(), it ends the mission without changing its status.

   if not tk.yesno( title, string.format( pre_accept[3], targetworld:name() ) ) then

      tk.msg( title, decline )

      misn.finish()
   end

   -- Set up mission information for the onboard computer and OSD.
   -- The OSD Title takes up to 11 signs.
   misn.setTitle( title )

   -- Reward is only visible in the onboard computer.
   misn.setReward( string.format( reward_desc, reward ) )

   -- Description is visible in OSD and the onboard computer, it shouldn't be too long either.
   misn.setDesc( string.format( misn_desc, targetworld_sys:name() ) )

   -- Set marker to a system, visible in any mission computer and the onboard computer.
   landmarker = misn.markerAdd( targetworld_sys, "low")


   -- Add mission
   -- At this point the mission gets added to the players active missions.
   misn.accept()

   -- Create two windows that show up after the player has accepted.
   -- Useful to explain further details.
   tk.msg( title, string.format( post_accept[1], targetworld_sys:name() ) )

-- Comment out this line to reveal the planet Samson is in.
--   tk.msg( title, targetworld:name() )

   -- Set up hooks.
   -- These will be called when a certain situation occurs ingame..
   hook.land("land")
   hook.takeoff("takeoff")
   hook.jumpin("jumpin")
end

   -- The function specified in the above hook.
function land ()
   -- Are we at our destination and did we find Samson?
   if planet.cur() == startworld and interrogated then
      -- Give the player their reward.
      player.pay( reward )

      -- Pop up a window that tells the player they finished the mission and got their reward.
      tk.msg( title, string.format(misn_accomplished) )

      -- Mark the mission as successfully finished.
      misn.finish(true)
   end
   
    -- We find Samson is trying to escape.
   if planet.cur() == targetworld and not found then
      tk.msg( title, string.format(misn_investigate) )
      found = true
   end

   if found and spawned and not interrogated then
      fail(1)
   end

end

function takeoff ()

   -- Set OSD after accepting mission and taking off. 
   if system.cur() == startworld_sys then
      OSDtable1[1] = OSDdesc1:format( targetworld_sys:name() )
      OSDtable1[2] = OSDdesc2
      misn.osdCreate( OSDtitle1, OSDtable1 )
      misn.osdActive(1)

   -- Set OSD after finding Samson and taking off. Spawn Samson in space. 
   elseif system.cur() == targetworld_sys and found and not spawned then
      OSDtable2[1] = OSDdesc3
      misn.osdDestroy()
      misn.osdCreate( OSDtitle2, OSDtable2 )
      spawn_samson( pos )
      spawned = true
   end
end

-- Moves to the next point upon jumping into target system
function jumpin ()
   if system.cur() == targetworld_sys and not found then
      misn.osdActive(2)
   end
   -- If you leave the system before interrgotating Samson
   if found and spawned and not interrogated then
      fail(1) 
   end
      
end

-- Spawn pilot Samson function
function spawn_samson( param )
   samson = pilot.addRaw( "Gawain", "mercenary" , targetworld , "Dummy")
   samson[1]:rename("Samson")
   samson[1]:control()
   samson[1]:runaway( player.pilot(), false )
   samson[1]:setHilight( true )
   hook.pilot( samson[1], "jump", "pilot_jump" )
   hook.pilot( samson[1], "death", "pilot_death" )
   hook.pilot( samson[1], "board", "pilot_board")
end


--Executes when you disable and board the ship
function pilot_board()
   tk.msg(board_title, board_msg)
   OSDtable3[1] = OSDdesc4:format( targetworld_sys:name() )
   misn.osdCreate( OSDtitle3, OSDtable3 )   
   interrogated = true
   misn.markerMove(landmarker, startworld_sys)
end

--Executes if he gets away
function pilot_jump ()
   if not interrogated then
      fail(1)
   end
end

--Executes if you kill him
function pilot_death()
   if not interrogated then
      fail(2)
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
