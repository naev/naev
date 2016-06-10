--[[Insane Businessman ]]--



-- For Translations
lang = naev.lang()
if lang == "es" then
elseif lang == "de" then 
else

--[[Text of mission]]--

npc_name = "A Businessman"
bar_desc = "A disheveled but familiar looking businessman."
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

function create ()

   startworld, startworld_sys = planet.cur()

   targetworld_sys = system.get("Cygnus")
   targetworld = planet.get("Jaan")

   if not misn.claim ( {targetworld_sys} ) then
      msn.finish()
   end

   reward = 10000

   misn.setNPC( npc_name, "neutral/unique/shifty_merchant" )
   misn.setDesc( bar_desc )
end


function accept ()

   tk.msg( title, pre_accept[1] )
   tk.msg( title, string.format( pre_accept[2], targetworld:name() ) )

   if not tk.yesno( title, string.format( pre_accept[3], targetworld:name(), targetworld_sys:name() ) ) then

      tk.msg( title, decline )

      misn.finish()
   end

   misn.setTitle( title )
   misn.setReward( string.format( reward_desc, reward ) )
   misn.setDesc( string.format( misn_desc, targetworld:name(), targetworld_sys:name() ) )

   landmarker = misn.markerAdd( targetworld_sys, "low")

   misn.accept()

   tk.msg( title, string.format( post_accept[1], targetworld:name() ) )

    OSDtable[1] = OSDdesc1:format( targetworld:name(), targetworld_sys:name() )
    OSDtable[2] = OSDdesc2:format( startworld:name(), startworld_sys:name() )
    misn.osdCreate( OSDtitle, OSDtable )

   hook.land("land")
   hook.takeoff("takeoff")
end

function land ()

   -- Are we back home and have we landed on target planet at least once?
   if planet.cur() == startworld and finished then

      player.pay( reward )
      tk.msg( title, string.format(misn_accomplished, reward) )

      misn.finish(true)

   end
   
    -- Checks to see if at target planet but has not landed there already.
    if planet.cur() == targetworld and not finished then
      tk.msg( title, string.format(misn_investigate) )
      finished = true
      misn.markerMove(landmarker, startworld_sys)
    end


end

function takeoff ()
   if finished then
      misn.osdActive(2)
   end
end
