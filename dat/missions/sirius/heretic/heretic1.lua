--[[misn title - the return]]
--[[after smuggling a small arms shipment to the an'ku system,
	the player is asked to deliver a message to a "shady character"
	on the wringer in the suna system.]]
	
include "numstring.lua"

lang = naev.lang()
--all the messages before the mission starts
bmsg = {}
bmsg[1] = [[You approach the tall man who is sitting at a table with papers and maps spread out before him. He glances up at you as you approach. You eye the maps warily, as they discombobulate you. That is, until you realize that they are attack strategy on key points of the %s infrastructure. The man stands up gracefully, and motions for you to sit.]]
bmsg[2] = [["Hello there," he begins to speak, "I'm sure you have quite a few questions. My name is Shaman and I am a proud  commander in what is known as the Nasin. We are currently terrorize the Sirius overlords that control %s, and as you can plainly see," he gusteres to the maps,"I have my hands full at the moment. Would you like to know more about the Nasin, or just move on to the work I need you to do?"]]
bmsg[3] = [["The mission is simple. The Nasin have their main base operating on..." Shaman looks around, realizing he might not want everyone to know where this place is. He reaches down and scratches something out on a piece of paper, and hands it to you. It simply reads "%s in %s". "I need a message delivered there. Our work is almost done here. Of course, we will pay you for this service. How does %s sound? Will you do it?"]]
bmsg[4] = [[Shaman takes a deep breath. "The Nasin were at one point all part of House Sirius, and believed solely in the teachings of Sirichana. We loved him, and our hearts were his. As all religions do at some point, however, the teachings of Sirichana became weighed down by the ideologies and agendas of man. Most people still accepted these teachings as straight from the mouth of Sirichana himself, but we, the Nasin, knew better."]]
bmsg[5] = [["We started a splinter religion, still trying to cooperate with the Sirii, but when the Serra felt threatened (as they well should have, I might add), they branded us as heretics and forced us out of Sirius space. At first, we didn't know what to do, but then, Jan Jusi pi Lawa came to lead us. He was the one who named us, the Nasin, which means "The Way" in an old earth language."]]
bmsg[6] = [[At this point, Shaman seems very excited, caught up in the moment. "It was he! He who led us to join our hands! He who led us to work together! He who led us to fight back against the oppressors! It was he! The very, the only, the True Voice of Sirichana!" 
   Shaman seems to realize just exactly where he is and what he is doing. All the patrons in the bar turn their heads to your table. A group of young fellows start clapping and then degrade into laughter.]]
bmsg[7] = [[Shaman coughs out an "excuse me" and looks at you, embarrassed. "It is wrong for me to get so caught up in such things. I suppose you'll want to know about the mission now."]]

--all the messages after the player lands on the target asset
emsg = {}
emsg[1] = [[You receive your clearance to land on %s, and begin your computer-assisted entry into the specified bay. Your not sure exactly what to expect, but you think that they've been expecting you. You successfully dock, and proceed into the hangar, ready for someone to come greet you. No one does. The hangar is oddly... empty. And that is when you notice it.]]
emsg[2] = [[There is an envelope, folded neatly, and laying squarely in the middle of the hangar. You go up to inspect it, and on the front, it simply says "%s". You snatch it up, and open it quickly.]]
emsg[3] = [["Hello %s,
			My sincerest apologies for being absent. I was... otherwise engaged. By the time you are reading this letter, our message will have already been moved from your ship. I appreciate the hard work, and have heard good things of you from both Ragnarok and Shaman. I do wish to speak with you in person, so please feel free to drop by the bar and have a drink. I'm sure you know the way. Your payment of %s was credited  to your account.
			Sincerly,
			Draga
			Secretary of Jan Jusi pi Lawa]]

--conversational options
option = {}
option[1] = "Tell me about the mission."
option[2] = "Tell me about the Nasin."

--random odds and ends
misn_title = "The Return"
npc_name = "Shaman"
bar_desc = "A tall man sitting at a table littered with papers."
misn_desc = "Deliver the message to %s in %s for Shaman."
osd = {}
osd[1] = "Fly to %s in the %s system and deliver the message."

function create()
   --this mission makes no system claims
   --create some mission variables
   nasin_rep = faction.playerStanding("Nasin")
   misn_tracker = var.peek("heretic_misn_tracker") --we use this at the end.
   reward = math.floor((10000+(math.random(5,8)*200)*(nasin_rep^1.315))*.01+.5)/.01 --using the actual reward algorithm now.
   targetasset, targetsystem = planet.get("The Wringer")
   playername = player.name()
   --set the mission stuff
   misn.setTitle(misn_title)
   misn.setReward(reward)
   misn.setNPC(npc_name,"neutral/male1")
   misn.setDesc(bar_desc)
   --format your strings, yo!
   bmsg[1] = bmsg[1]:format(planet.cur():name())
   bmsg[2] = bmsg[2]:format(planet.cur():name())
   bmsg[3] = bmsg[3]:format(targetasset:name(),targetsystem:name(),numstring(reward))
   emsg[1] = emsg[1]:format(targetasset:name())
   emsg[2] = emsg[2]:format(playername)
   emsg[3] = emsg[3]:format(playername,numstring(reward))
   osd[1] = osd[1]:format(targetasset:name(),targetsystem:name())
   misn_desc = misn_desc:format(targetasset:name(),targetsystem:name())
end

function accept()
   spk_choice = tk.choice(misn_title,bmsg[2],option[1],option[2]) --using tk.choice felt more natural than just a yes or no.
   if spk_choice == 2 then
      faction.modPlayer("Nasin",3) --a little reward for actually wanting the storyline (read: nasin loyalty).
      tk.msg(misn_title,bmsg[4])
      tk.msg(misn_title,bmsg[5])
      tk.msg(misn_title,bmsg[6])
      tk.msg(misn_title,bmsg[7])
   end
   if not tk.yesno(misn_title,bmsg[3]) then
      misn.finish(false)
   end
   misn.setDesc(misn_desc)
   misn.accept()
   misn.markerAdd(targetsystem,"plot")
   misn.osdCreate(misn_title,osd)
   misn.osdActive(1)
   message = misn.cargoAdd("Message",0)
   hook.land("landing")
end

function landing()
   if planet.cur() == targetasset then
      tk.msg(misn_title,emsg[1])
      tk.msg(misn_title,emsg[2])
      tk.msg(misn_title,emsg[3])
      player.pay(reward)
      misn.cargoRm(message)
      misn_tracker = misn_tracker + 1
      faction.modPlayer("Nasin",3) --once again, the nasin like the fact that we are helping the nasin.
      var.push("heretic_misn_tracker",misn_tracker)
      misn.osdDestroy()
      misn.finish(true)
   end
end

function abort()
   misn.osdDestroy()
   misn.finish(false)
end
