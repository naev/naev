--[[Insane Businessman ]]--



-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then -- Spanish version of the texts would follow
elseif lang == "de" then -- German version of the texts would follow 
else -- default English text

--[[This section is used to define lots of variables holding all the various texts. ]]--

npc_name = "A Businessman"
bar_desc = "A disheveled looking businessman"
title = "Insane Businessman"
pre_accept = {}

pre_accept[1] = [[As soon as your eyes set on the disheveled man sitting in a dark corner throwing back drink after drink, you realize who he is. The well known rich businessman named Daniel Crumb. He signals you to come talk to him so you carefully make your way over to his booth.

"I've heard that you are a very good "freelancer", shall we say? Is this true?" He asks, leaning over to you with a drink in his hand. 

"I guess you could say that," you reply, confidently. 
]] 

pre_accept[2] = [["Good. I need someone to fly over to the %s system to, well, "investigate" some people. Is this something you would be interested in doing?"

"Depends," you reply, then ask "What are the details?"]]

pre_accept[3] = [["Oh, nothing too dangerous. Just some hippies planning a sit in at one of my properties. Nothing too dangerous like I said."

"Hippies? Property?"

"I own some property out in %s in the %s system. Real nice houses and hotels. I sold a house to an old lady but I guess she couldn't afford her mortgage anymore, so when she stopped paying, I kicked her out.  These hippies got involved. I think they're planning a sit-in. All I need you to do is fly over there and get me information on the what they're up to. Can you do that, I guarantee to make it worth your while?" ]]

decline = [["Ah, well some other time maybe..."]]

misn_desc = "Investigate protesters in %s in the %s system"
reward_desc = "%s credits on completion."
post_accept = {}
post_accept[1] = [["Great. Fly over to %s and investigate the hippy protesters."]]

misn_investigate = [[Upon arrival, first thing you do is check the house. It seems to be vacant. Not a soul in sight. You ask some people in the neighborhood and nobody knows anything about any hippy protesters. All they know is that the old lady in the house was evicted over a month ago. She left quietly. This is truly strange. Could Crumb have been misinformed about the hippies?]]

misn_accomplished = [[Crumb awaits you at the landing pad. As soon as you exit your ship, he eagerly comes up to you asking what you uncovered. You frown and take a deep breath before telling him you were not able to find any group of hippy protesters planning a sit in. The property was vacant, the old woman had left. It appears to you that Mr. Crumb was misinformed. 

"Well, no matter, I'll pay you anyway." He says. "I have another lead about the people trying to ruin me."

"Ruin you?" You ask. 

"Yes, ruin me. I'll tell you more in a bit. Meet me at the bar."
]]


-- OSD

OSDtitle = "Investigate Protesters"
OSDdesc1 = "Go to %s in the %s system and investigate protesters on Crumb's property."
OSDdesc2 = "Return to %s in the %s system and report what you found to Crumb."
OSDtable = {}

end



--[[
After the texts follow the functions.
There are bascially two types, those defined inside this file and the API functions.
]]--

function create ()
   -- This will get called when the conditions in mission.xml are met (or when the mission is initiated from another script).
   -- It is used to set up mission variables.

   -- Get the planet and system at which we currently are.
   startworld, startworld_sys = planet.cur()

   -- Set our target system and planet.
   targetworld_sys = system.get("Cygnus")
   targetworld = planet.get("Jaan")

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

   -- Introductory text when you approach the NPC

   tk.msg( title, pre_accept[1] )
   tk.msg( title, string.format( pre_accept[2], targetworld:name() ) )

   -- Show a yes/no dialog with a title and a text.
   -- If the answer is no the following block will be executed.
   -- In this case, misn.finish(), it ends the mission without changing its status.

   if not tk.yesno( title, string.format( pre_accept[3], targetworld:name(), targetworld_sys:name() ) ) then

      tk.msg( title, decline )

      misn.finish()
   end

   -- Set up mission information for the onboard computer and OSD.
   -- The OSD Title takes up to 11 signs.
   misn.setTitle( title )

   -- Reward is only visible in the onboard computer.
   misn.setReward( string.format( reward_desc, reward ) )

   -- Description is visible in OSD and the onboard computer, it shouldn't be too long either.
   misn.setDesc( string.format( misn_desc, targetworld:name(), targetworld_sys:name() ) )
   -- Set marker to a system, visible in any mission computer and the onboard computer.
   landmarker = misn.markerAdd( targetworld_sys, "low")


   -- Add mission
   -- At this point the mission gets added to the players active missions.
   misn.accept()

   -- Create a windowa that shows up after the player has accepted.
   -- Useful to explain further details.
   tk.msg( title, string.format( post_accept[1], targetworld:name() ) )

   -- Set up hooks.
   -- These will be called when a certain situation occurs ingame.
   -- See http://api.naev.org/modules/hook.html for further hooks.
   hook.land("land")
   hook.takeoff("takeoff")
end

   -- The function specified in the above hook.
function land ()
   -- Are we back home and have we investigated the protesters?
   if planet.cur() == startworld and finished then
      -- Give the player their reward.
      player.pay( reward )

      -- Pop up a window that tells the player they finished the mission and got their reward.
      tk.msg( title, string.format(misn_accomplished, reward) )

      -- Mark the mission as successfully finished.
      misn.finish(true)
   end
   
    -- Are we at the target planet? What do we see there.
    if planet.cur() == targetworld then
      tk.msg( title, string.format(misn_investigate) )
      finished = true
      misn.markerMove(landmarker, startworld_sys)
    end


end

-- The takeoff hook function. Sets OSD. 
function takeoff ()
   if finished then
      misn.osdActive(2)
   else
      OSDtable[1] = OSDdesc1:format( targetworld:name(), targetworld_sys:name() )
      OSDtable[2] = OSDdesc2:format( startworld:name(), startworld_sys:name() )
      misn.osdCreate( OSDtitle, OSDtable )
   end
end


-- This will be called when the player aborts the mission in the onboard computer.
function abort ()
   -- Mark mission as unsuccessfully finished. It won't show up again if this mission is marked unique in mission.xml.
   misn.finish( false )
end

