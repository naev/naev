--[[Insane Businessman ]]--



-- For Translations
lang = naev.lang()
if lang == "es" then
elseif lang == "de" then 
else

--[[Text of mission]]--

npc_name = "A Businessman"
bar_desc = "An unkempt familiar looking businessman."
title = "Insane Businessman"
pre_accept = {}

pre_accept[1] = [[You see an unfastidious man sitting in a dark corner booth. You can't help but think he looks familiar as you watch him throw back drink after drink. Wondering where you might have seen him before, it finally hits you. It's the notorious, well known, and very rich businessman named Daniel Crumb.

You tell the bartender you work as a freelancer and would like to meet Crumb. He sends a busboy over who whispers something to Crumb and subsequently Crumb signals you to come over to the booth. As you approach the booth, you reach out your arm to shake his hand.  

"So, you want to work for old Mr. Crumb?" He says, almost mockingly.

"Well, I have a pretty good ship and I can do just about anything."

"Ok. Let's try something easy."
]] 

pre_accept[2] = [["I own some property out in the %s system. I need someone to fly over there to, well, "investigate" some people. Is this something you would be interested in doing?"

"Sure," you reply, then ask "What are the details?"]]

pre_accept[3] = [["Oh, nothing too dangerous. Just some protesters planning a sit in at a house I foreclosed on."

"Protesters? Foreclosure?"

"Yes, like I said, I own property over there. Real nice houses and hotels. I sold a house to an old lady but I guess she couldn't afford her mortgage anymore, so when she stopped paying, I kicked her out. Some protesters got involved and now they don't want to leave. All I need you to do is fly over to the property and get me some information on the what they're up to. Let's see if you can do that, then maybe I'll send you on a more interesting mission. There will be compensation, of course. So what do you say, you still want to work for Crumb?" ]]

decline = [["Ah, well the offers open for you whenever you want. Just look for me here."]]

misn_desc = "Investigate protesters in %s in the %s system"
reward_desc = "%s credits on completion."
post_accept = {}
post_accept[1] = [["Great. Fly over to %s and investigate the protesters."]]

misn_investigate = [[Upon arrival, first thing you do is check the house. It seems to be vacant. Not a soul in sight. You ask some people in the neighborhood and nobody knows anything about any protesters. All they know is that the old lady in the house was evicted over a month ago. She left quietly. This is truly strange. Could Crumb have been misinformed about the protesters?]]

misn_accomplished = [[Crumb awaits you at the spaceport. As you exit your ship, he eagerly rushes up and asks what you found. You frown and take a deep breath before admitting that you uncovered nothing. The property was vacant, the old woman had left. 

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
      abort(false)
   end

   reward = 10000

   misn.setNPC( npc_name, "neutral/unique/shifty_merchant" )
   misn.setDesc( bar_desc )
end


function accept ()

   tk.msg( title, pre_accept[1] )
   tk.msg( title, string.format( pre_accept[2], targetworld_sys:name() ) )

   if not tk.yesno( title, string.format( pre_accept[3], targetworld:name(), targetworld_sys:name() ) ) then
      tk.msg( title, decline )
      abort(false)
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

   landhook = hook.land("land")
   takeoffhook = hook.takeoff("takeoff")
end

function land()

   -- Are we back home and have we landed on target planet at least once?
   if planet.cur() == startworld and finished then
      player.pay( reward )
      tk.msg( title, string.format(misn_accomplished, reward) )      
      abort(true)

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

function abort(status)
   hooks = { landhook, takeoffhook }
   for _, j in ipairs( hooks ) do
      if j ~= nil then
         hook.rm(j)
      end
   end   
   misn.finish(status)
   misn.osdDestroy()
end
