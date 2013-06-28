--[[misn title - the egress]]
--[[this mission begins with the frenetic nasin wanting to escape
	the wringer due to being overwhelmed by house sirius. the player
	loads up with as many nasin as their vessel will carry, and takes
	them to seek refuge in the ingot system on planet ulios, where they
	begin to rebuild and plan.... (ominous music).
	this mission is designed to be the end of part 1, and is supposed
	to be very hard, and slightly combat oriented, but more supposed to
	involve smuggling elements.]]
	
lang = naev.lang()

--beginning messages
bmsg = {}
bmsg[1] = [[You run up to Draga, who has a look of desperation on his face. He talks to you with more than a hint of urgency.
   "We need to go, now. The Sirius are overwhelming us, they're about to finish us off. Will you take me and as many Nasin as you can carry to %s in %s? I swear, if you abort this and jettison those people into space, I will hunt you down and destroy you."]]
bmsg[2] = [[You lead Draga out of the bar and into your landing bay. Refugees are milling about, hoping to get aboard some ship or another. Draga asks you what your tonnage is, and begins directing people on board. Panic erupts when gunfire breaks out on the far end of the bay.]]
bmsg[3] = [[As the gunfire gets nearer, Draga yells at you to get the ship going and take off. As you get in the cabin and begin rising into the air you see Draga running back into the bay to help a woman run with her children to a small inter-planetary skiff. Some Sirius soldiers run and catch up with him, giving him three shots in the chest. The last thing you see as you take off was Draga in a pool of blood, and the woman being dragged off by the soldiers. You rocket out of there, and begin prepping to get those people to safety.]]

--ending messages
emsg = {}
emsg[1] = [[You land on %s, and open the bay doors. You are amazed at how many people Draga had helped get into the cargo hold. You help out the last few people, who are grateful to you, and walk down into the bay. A man walks up to you.]]
emsg[2] = [[The man offers his hand, and begins to speak. 
   "Hello, my name is Jimmy. Thank you for helping all these people." He gestures to the refugees, who are being helped by some officials. "I am grateful. I've heard about you, from Draga, and I will be forever in your debt. All I can manage right now is this, so consider me in your debt."
   He presses a credit chip in your hand, and walks away to help some refugees.]]

--mission osd
osd = {}
osd[1] = "Fly the refugees to %s in the %s system."

--random odds and ends
abort_msg = [[You decide that this mission is just too much. You open up the cargo doors and jettison the %d people out into the cold emptiness of space. The Nasin, and the Sirius, will hate you forever, but you did what you had to do.]]
misn_title = "The Egress"
npc_name = "Draga"
bar_desc = "Draga is running around, helping the few Nasin in the bar to get stuff together and get out."
misn_desc = "Assist the Nasin refugees by flying to %s in %s, and unloading them there."

function create()
   --this mission make no system claims.
   --initalize your variables
   nasin_rep = faction.playerStanding("Nasin")
   misn_tracker = var.peek("heretic_misn_tracker")
   reward = math.floor((10000+(math.random(5,8)*200)*(nasin_rep^1.315))*.01+.5)/.01
   homeasset = planet.cur()
   targetasset, targetsys = planet.get("Ulios") --this will be the new HQ for the nasin in the next part.
   free_cargo = pilot.cargoFree(pilot.player())
   people_carried =  (16 * free_cargo) + 7 --average weight per person is 62kg. one ton / 62 is 16. added the +7 for ships with 0 cargo.
   --set some mission stuff
   misn.setNPC(npc_name,"neutral/thief2")
   misn.setDesc(bar_desc)
   misn.setTitle = misn_title
   misn.setReward = reward
   --format your strings, yo!
   bmsg[1] = bmsg[1]:format(targetasset:name(),targetsys:name())
   emsg[1] = emsg[1]:format(targetasset:name())
   osd[1] = osd[1]:format(targetasset:name(),targetsys:name())
   abort_msg = abort_msg:format(people_carried)
   misn_desc = misn_desc:format(targetasset:name(),targetsys:name())
end

function accept()
   --inital convo. Kept it a yes no to help with the urgent feeling of the situation.
   if not tk.yesno(misn_title,bmsg[1]) then
      misn.finish ()
   end
   misn.accept()
   misn.setDesc(misn_desc)
   misn.osdCreate(misn_title,osd)
   misn.osdActive(1)
   player.allowSave(false) -- so the player won't get stuck with a mission he can't complete.
   tk.msg(misn_title,bmsg[2])
   tk.msg(misn_title,bmsg[3])
   --convo over. time to finish setting the mission stuff.
   misn.markerAdd(targetsys,"plot")
   refugees = misn.cargoAdd("Refugees",free_cargo)
   player.takeoff()
   --get the hooks.
   hook.takeoff("takeoff")
   hook.jumpin("attacked")
   hook.jumpout("lastsys")
   hook.land("misn_over")
end

function takeoff()
   pilot.add("Sirius Assault Force",sirius,vec2.new(rnd.rnd(-450,450),rnd.rnd(-450,450))) --left over fleets from the prior mission.
   pilot.add("Sirius Assault Force",sirius,vec2.new(rnd.rnd(-450,450),rnd.rnd(-450,450)))
   pilot.add("Sirius Assault Force",sirius,vec2.new(rnd.rnd(-450,450),rnd.rnd(-450,450)))
   pilot.add("Nasin Sml Civilian",nil,homeasset) --other escapees.
   pilot.add("Nasin Sml Civilian",nil,homeasset)
   pilot.add("Nasin Sml Civilian",nil,homeasset)
   pilot.add("Nasin Sml Attack Fleet",nil,homeasset) --these are trying to help.
   pilot.add("Nasin Sml Attack Fleet",nil,homeasset)
end

function lastsys()
   last_sys_in = system.cur()
end

function attacked() --several systems where the sirius have 'strategically placed' an assault fleet to try and kill some nasin.
   dangersystems = {
   system.get("Neon"),
   system.get("Pike"),
   system.get("Vanir"),
   system.get("Aesir"),
   system.get("Herakin"),
   system.get("Eiderdown"),
   system.get("Eye of Night"),
   system.get("Lapis"),
   system.get("Ruttwi"),
   system.get("Esker"),
   system.get("Gutter")
   }
   for i,sys in ipairs(dangersystems) do
      if system.cur() == sys then
         pilot.add("Sirius Assault Force",sirius,vec2.new(rnd.rnd(-300,300),rnd.rnd(-300,300)))
      end
   end
   local chance_help,chance_civvie = rnd.rnd(1,3),rnd.rnd(1,3) --attack fleet and civvies are meant as a distraction to help the player.
   if chance_help == 1 then
      pilot.add("Nasin Sml Attack Fleet",nil,last_sys_in)
   end
   for i = 1,chance_civvie do
      pilot.add("Nasin Sml Civilian",nil,last_sys_in)
   end
end

function misn_over() --arent you glad thats over?
   if planet.cur() == planet.get("Ulios") then
      tk.msg(misn_title,emsg[1]) --introing one of the characters in the next chapter.
      tk.msg(misn_title,emsg[2])
      player.pay(reward)
      misn.cargoRm(refugees)
      misn_tracker = misn_tracker + 1
      faction.modPlayer("Nasin",8) --big boost to the nasin, for completing the prologue
      faction.modPlayer("Sirius",-5) --the sirius shouldn't like you. at all.
      var.push("heretic_misn_tracker",misn_tracker)
      misn.osdDestroy()
      player.allowSave(true)
      misn.finish(true)
   end
end

function abort()
   tk.msg(misn_title,abort_msg)
   var.push("heretic_misn_tracker",-1) --if the player jettisons the peeps, the nasin will not let the player join there ranks anymore.
   misn.osdDestroy()
   player.allowSave(true)
   misn.finish(true)
end
