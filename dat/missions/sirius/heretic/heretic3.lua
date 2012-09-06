--[[misn title - the assault]]
--[[in this mission, the wringer is assaulted by a full assault fleet
	sent in by the threatened sirius. the player attempts to defend,
	when is instead ordered back to the wringer to escape sirius controlled
	space. thanks to nloewen and viashimo for help!]]
	

include "numstring.lua"
	
lang = naev.lang()
--beginning messages and choices
bmsg = {}
bmsg[1] = [[Draga is sitting at a table with a couple other people who appear to be official military types. They look at you as you approach. Draga stands and greets you.
   "Hello, %s. We have a situation, and we need your help."]]
bmsg[2] = [[Draga sits back down with the other officials.
   "Sirius is officially considering us a threat. As such, we need you to help us defend our system. Our goal here isn't to completely wipe out the Sirius threat, but rather just to drive them off and show them that we mean business. We want them to feel it."]]
bmsg[3] = [[Draga leans back and smiles ruefully. 
   "So, you will be outnumbered, outgunned, and officially declared an enemy of the state. Want in? Please note, however, that if you abort the mission, or jump out-system, you will be dismissed. Permanently."]]
--messages for the choice
bmsgc = {}
bmsgc[1] = [[He snorts derisively at your question.
   "Why, I mean you will no longer be welcome amongst the Nasin. At least, as a pilot."]]
bmsgc[2] = [[Sorrow fills Draga's eyes at the question.
   "Unfortunately, House Sirius and those claiming to represent Sirichana feel that we are a band of heretics out to destroy the Sirius way of life. We aren't out to destroy it, but rather to bring it to completion. They have decided that we need to be eliminated."]]
bmsgc[3] = [[Draga eyes you. You can tell he doesn't like the question.
   "You really like the money, %s? You know, I'd much prefer it if you were just loyal enough to not worry about the money. We will always pay you well. We take care of our own. We can pay you %s. I hope its good enough"]]
bmsgc[4] = [[Draga is quite excited at the news, and the surrounding officers mumble their approval. "Great! You should hurry up and take off, we are expecting them any second now. We already have one defensive element in space."]]
bmsgc[5] = [[Obviously annoyed beyod belief, Draga doesn't say a word. He makes a shooing guesture with his hand, and doesn't even look as you go.]]
draga_chooser = [[Draga looks at you impatiently. "Any more questions or can we get going?"]]
choice = {}
choice[1] = "What do you mean, permanently?"  --this opens up dialog bmsg[4]
choice[2] = "Why are they attacking?" --jumps to dialog bmsg[5]
choice[3] = "What are you paying?" --jumps to dialog bmsg[6]
choice[4] = "I'm in." --jumps to dialog bmsg[7], and starts mission.
choice[5] = "Gimme a minute. I'll be right back." --jumps to dialog bmsg[8]

--ending messages
emsg = {}
emsg[1] = [[As you land, you see people running about frantically. Most look as though they've had some sort of military training, but there are a few scrambling civilians as well. You land, and as soon as your feet hit the deck the now familiar face of Draga comes up to you begins to speak.
   "Get your crap together, %s, and meet me in the bar as soon as you can. We need you. I've already transferred payment." At this, he runs off to helps a group of elderly citizens who are struggling against the tide of people.]]

--misn osd
osd = {}
osd[1] = "Defend %s against the oncoming assault!"
osd[2] = "Return to %s."
--random odds and ends
misn_title = "The Assault"
npc_name = "Draga"
bar_desc = "The familiar form of Draga is at a table with some officers. They look busy."
return_to_base_msg = [[A clear voice booms in your ship through the comm system.
   "%s! We need you to return to %s immediately! We are being overwhelmed and are evacuating! Please hurry!" The voice cuts out in a tumult of static.]]
p_landing = [[Draga appears with three armed Nasin carrying some serious weaponry. 
   He points at you and says "I have been more than accomadating with you. Now, you are dismissed. Take care of your business, and leave. And if you don't, my friends here are more than willing to collect that bounty."
   Draga strides away, exuding annoyance and rage.]] 
oos = [[A voice comes into your comm system. "We need you back in the system now! Hurry!"]]
oos_failure = [[A scratchy voice from what sounds like very far away cut in on your comms priority channel. 
   "We needed you! We are being overrun by Sirius and we aren't gonna make it. Don't bother coming back to us. Ever." 
   The voice cuts out, and you feel like you've made a horrible mistake.]]
misn_desc = [[A Sirius assault fleet has just jumped into %s. Destroy this fleet. WARNING: DO NOT JUMP OUT-SYSTEM OR LAND ON ANY ASSETS.]]
time_to_come_home = [[Your comm squeaks as the voice of Draga comes onto the channel. "%s! This is a lot larger of an assault than we thought. There is no way that we can handle this. We need you to get back to the station now! We are evacuating!" The comm goes silent, and you start heading back.]]

function create()
   --this mission makes one mission claim, in suna.
   --initialize your variables
   nasin_rep = faction.playerStanding("Nasin")
   misn_tracker = var.peek("heretic_misn_tracker")
   reward = math.floor((10000+(math.random(5,8)*200)*(nasin_rep^1.315))*.01+.5)/.01
   planding = 0
   homeasset, homesys = planet.cur()
   msg_checker = 0
   --set the mission stuff
   if not misn.claim(homesys) then
      tk.msg("debug","system not claimed! mission aborting.")
      misn.finish(false)
   end
   misn.setReward( reward )
   misn.setTitle( misn_title )
   misn.setNPC(npc_name,"neutral/thief2")
   misn.setDesc(bar_desc)
   --format your strings, yo!   
   bmsg[1] = bmsg[1]:format(player.name())
   bmsgc[3] = bmsgc[3]:format(player.name(),numstring(reward))
   emsg[1] = emsg[1]:format(player.name())
   return_to_base_msg = return_to_base_msg:format(player.name(),homeasset:name())
   osd[1] = osd[1]:format(homeasset:name())
   osd[2] = osd[2]:format(homeasset:name())
   misn_desc = misn_desc:format(homesys:name())
   time_to_come_home = time_to_come_home:format(player.name())
end

function accept()
   tk.msg(misn_title,bmsg[1])
   tk.msg(misn_title,bmsg[2])
   while true do --another complicated convo!
      if checker == nil then
         chooser = tk.choice(misn_title,bmsg[3],choice[1],choice[2],choice[3],choice[4],choice[5])
      else
         chooser = tk.choice(misn_title,draga_chooser,choice[1],choice[2],choice[3],choice[4],choice[5])
      end
      if chooser == 1 or chooser == 2 or chooser == 3 or chooser == 4 then
         tk.msg(misn_title,bmsgc[chooser])
         if chooser == 3 and rep_check == nil then
            faction.modPlayer("Nasin",-5) --the nasin prefer loyalty to them over money. rep hurt if player asks about pay.
            rep_check = 1 --makes sure the rep hit only comes once.
         end
      if chooser == 4 then
         break
         end
      else
         tk.msg(misn_title,bmsgc[chooser])
         misn.finish()
         break
      end
      checker = "you been through one time, yo!" --makes sure the intro message to the convos only comes in once
   end

   misn.setDesc(misn_desc) --convo is over! time to set the last of the mission stuff
   misn.accept()
   misn.markerAdd(homesys,"plot")
   misn.osdCreate(misn_title,osd)
   misn.osdActive(1)

   --hook time.
   hook.takeoff("takeoff")
   hook.jumpin("out_sys_failure")
   hook.land("return_to_base")
end

function takeoff() --for when the player takes off from the wringer.
   pilot.clear() --clearing out all the pilots, and
   pilot.toggleSpawn("Sirius",false) --making the sirius not spawn. I want the assault fleet the only sirius in there.
   deathcounter = 0 -- Counts destroyed Nasin ships.
   sirius_be_serious = pilot.add("Sirius Assault Force",sirius,system.get("Herakin"))
   
   for _,p in ipairs(sirius_be_serious) do
      p:setHilight()
      p:setNoJump()
      p:setNoLand()
      p:setHostile() --just in case. makes thing easier.
   end
   
   de_fence = pilot.add("Nasin Med Defense Fleet",nil,homeasset)
   de_fence_2 = pilot.add("Nasin Med Defense Fleet",nil,vec2.new(rnd.rnd(25,75),rnd.rnd(100,350)))
   
   for _,p in ipairs(de_fence) do
      p:setNoJump()
      p:setNoLand()
      p:setFriendly() --the green more clearly defines them as allies.
      hook.pilot(p,"death","death")
   end
   
   for _,p in ipairs(de_fence_2) do
      p:setNoJump()
      p:setNoLand()
      p:setFriendly( true )
      hook.pilot(p,"death","death")
   end
   hook.timer(90000,"second_coming") --i wanted the player to feel some hope that he'd win, but have that hope come crashing down.
   hook.timer(97000,"second_coming")
   hook.timer(145000,"second_coming")
end

function death(p)
   deathcounter = deathcounter + 1
   if deathcounter == 9 then --9 ships is all the ships in the first fleet minus the 2 cruisers and the carrier. might adjust this later.
      flee()
   end
end

function flee()
   tk.msg(misn_title,return_to_base_msg)
   returnchecker = true --used to show that deathcounter has been reached, and that the player is landing 'just because'
   misn.osdActive(2)
   tk.msg(misn_title,time_to_come_home)
   -- Send any surviving Nasin ships home.
   for _, j in ipairs(de_fence) do
      if j:exists() then
         j:control()
         j:land(homeasset)
         j:hookClear() -- So we don't trigger death() again.
      end
   end
   for _, j in ipairs(de_fence_2) do
      if j:exists() then
         j:control()
         j:land(homeasset)
         j:hookClear() -- So we don't trigger death() again.
      end
   end
end

function out_sys_failure() --feel like jumping out? AWOL! its easier this way. trust me.
   tk.msg(misn_title,oos_failure) 
   misn.osdDestroy()
   misn.finish(false)
end

function second_coming()
   sirius_be_serious_2 = pilot.add("Sirius Assault Force",sirius,system.get("Herakin"))
   for i,p in ipairs(sirius_be_serious_2) do
      table.insert(sirius_be_serious,p) --inserting into the original table, for the death function.
      p:setHilight()
      p:setNoJump()
      p:setNoLand()
      p:setHostile(true)
   end
end

function return_to_base()
   if not returnchecker then --feel like landing early? AWOL!
      tk.msg(misn_title,p_landing)
      misn.osdDestroy()
      misn.finish(false) --mwahahahahaha!
   else
      player.pay(reward)
      tk.msg(misn_title,emsg[1])
      misn_tracker = misn_tracker + 1
      faction.modPlayer("Nasin",4)
      faction.modPlayer("Sirius",-5)
      var.push("heretic_misn_tracker",misn_tracker)
      misn.osdDestroy()
      misn.finish(true)
   end
end

function abort()
   misn.osdDestroy()
   misn.finish(false)
end
