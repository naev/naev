--[[misn title - the patrol]]
--[[in this mission, the player will be guarding the "high command" of the
	nasin, the wringer/suna. house sirius is sending in recon parties.
	the players job is to take out any and all sirius in the system.]]

include "numstring.lua"

lang = naev.lang()

bmsg = {}
--beginning messages
bmsg[1] = [[You walk up to an intimidating man dressed smartly in cool, dark black business attire. He has a large smile spread across his face. 
   "Ahh, so your the %s everyone has been talking about. Naught but a glorified delivery boy if you ask me. Still, if you wish to help us out and prove yourself as more than a pirate, I'd be more than happy to oblige." He grins cooly, expecting an answer.]]
bmsg[2] = [[Draga snorts impatiently. "Well, do you take the mission or what?"]]
choice = {}
choice[1] = "Tell me more about the Nasin."
choice[2] = "What's the job?"
choice[3] = "I'm in. Where do I sign up?"
choice[4] = "Sounds risky. Give me some time."
chooser = {}
chooser[1] = [[He looks at you, betraying a little emotion. "The Nasin are a pure piece of glass. When light shines through glass, the light is only as pure as the glass itself. If the glass is dirty, then the light is distorted, and doesn't come through correctly. If the glass is mishappen or broken, the light may not filter through at all. We, the Nasin, are the purest glass there is, and House Sirius has become corrupt. We exist to see its downfall."]]
chooser[2] = [[Draga motions you in closer. "We have reason to believe that %s is about to be attacked. We are expecting Sirius to send in recon elements any STU now. We want you to handle that, as you see fit. Keep them away from the station. Better yet, kill them. We can pay you %s. We do ask that you stay in-system and off-planet until the mission is complete, otherwise you'll be considered AWOL, which means you're fired."]]
chooser[3] = [[Draga looks triumphant, but only for an instant. "Great. You should get going, we are expecting them at any second. Good luck, and godspeed."]]
chooser[4] = [[You brace yourself, as Draga appears ready to attack. He waves his arms about in obvious anger. "Great! I knew you were a waste of time. Well, if you decide to outgrow your diapers, I'll be right here waiting for you."
   You walk away insulted, but strangely curious.]]
--message at the end
emsg_1 = [[You land, having destroyed the small recon force. Draga is in the hangar, waiting for you.
   "Good job on proving yourself more than a delivery boy! That wasn't so bad, was it? Here's your payment, meet me in the bar soon."]]
--mission osd
osd = {}
osd[1] = "Destroy the Sirius fighter element."
osd[2] = "Element destroyed. Land on %s."
--random odds and ends
misn_title = "The Patrol"
npc_name = "Draga"
bar_desc = "An imposing man leans against the bar easily, looking right at you."
not_finished = "Draga seems to swoop in out of nowhere as soon as you land. \"You are not finished! Get back up there quickly!\""
chronic_failure = [[Draga swoops in. His nostrils are flaring, and he is obviously annoyed.
   "Apparently you have better things to do. Get out of here. I don't want to see your face anymore."
   You consider yourself fired.]]
doom_clock_msg = [[A scratchy voice jumps in on your comms priority channel. 
   "You jumped out of the system! We are being scanned by the enemy! We need you back to take care of this situation now, or you are AWOL and are not getting paid!!" 
   The voice and the scratch cuts out.]]
out_sys_failure_msg = [[Your comm station flares up with a scratchy, obviously-from-far-away noise. A voice is heard through it.
   "%s! We told you we needed you to stay in system! Apparently you have more important things to do. So get lost, kid! We'll take care of ourselves." The static cuts out, and you consider yourself fired.]]
misn_desc = "Destroy the Sirius recon element that flew into %s. WARNING: DO NOT JUMP OUT-SYSTEM OR LAND ON THE PLANET PREMATURELY."

function create()
   --this mission does make one system claim, in suna.
	--initialize the variables
   homeasset, homesys = planet.cur()
   if not misn.claim(homesys) then
      misn.finish(false)
   end
   nasin_rep = faction.playerStanding("Nasin")
   misn_tracker = var.peek("heretic_misn_tracker")
   playername = player.name()
   reward = math.floor((10000+(math.random(5,8)*200)*(nasin_rep^1.315))*.01+.5)/.01
   chronic = 0
   finished = 0
   takeoff_counter = 0
   draga_convo = 0
   deathcount = 0
   --set the mission stuff
   misn.setTitle(misn_title)
   misn.setReward(numstring(reward) .. "credits")
   misn.setNPC(npc_name,"neutral/thief2")
   misn.setDesc(bar_desc)
   --format the messages
   bmsg[1] = bmsg[1]:format(playername)
   chooser[2] = chooser[2]:format(homeasset:name(),numstring(reward))
   osd[2] = osd[2]:format(homeasset:name())
   out_sys_failure_msg = out_sys_failure_msg:format(playername)
   misn_desc = misn_desc:format(homesys:name())
end

function accept()
   while true do --this is a slightly more complex convo than a yes no. Used break statements as the easiest way to control convo.
      if first_talk == nil then --used the first talk one only the first time through. This way, the npc convo feels more natural.
         draga_convo = tk.choice(misn_title,bmsg[1],choice[1],choice[2],choice[3],choice[4])
         first_talk = 1
      else
         draga_convo = tk.choice(misn_title,bmsg[2],choice[1],choice[2],choice[3],choice[4])
      end
      if draga_convo == 1 or draga_convo == 2 then
         tk.msg(misn_title,chooser[draga_convo])
      elseif draga_convo == 3 then
         tk.msg(misn_title,chooser[draga_convo])
      break
      else
         tk.msg(misn_title,chooser[draga_convo] )
         misn.finish()
      break
      end
   end
   misn.setDesc(misn_desc)
   misn.accept()
   misn.markerAdd(homesys,"plot")
   misn.osdCreate(misn_title,osd)
   misn.osdActive(1)
   hook.takeoff("takeoff")
   hook.jumpin("out_sys_failure")
   hook.land("land")
end

function takeoff()
   pilot.clear()
   pilot.toggleSpawn("Sirius",false) --the only sirius i want in the system currently is the recon force
   recon = pilot.add("Sirius Recon Force",nil,system.get("Herakin"))
   attackers = pilot.add("Nasin Sml Attack Fleet",nil,homeasset) --a little assistance
   n_recon = #recon --using a deathcounter to track success
   for i,p in ipairs(recon) do
      p:setHilight(true)
      p:setNoJump(true) --dont want the enemy to jump or land, which might happen if the player is slow, or neutral to Sirius.
      p:setNoLand(true)
      p:setHostile(true)
   end
   for i,p in ipairs(attackers) do
      p:setNoJump(true)
      p:setNoLand(true)
      p:setFriendly(true)
   end
   hook.pilot(nil,"death","death")
end

function death(p)
   for i,v in ipairs(recon) do
      if v == p then
         deathcount = deathcount + 1
      end
   end
   if deathcount == n_recon then --checks if all the recon pilots are dead.
      misn.osdActive(2)
      finished = 1
   end
end

function land()
   if finished ~= 1 then
      tk.msg(misn_title,chronic_failure) --landing pre-emptively is a bad thing.
      misn.osdDestroy()
      misn.finish(false) 
   elseif planet.cur() == homeasset and finished == 1 then
      tk.msg(misn_title,emsg_1)
      player.pay(reward)
      misn_tracker = misn_tracker + 1
      faction.modPlayer("Nasin",4)
      faction.modPlayer("Sirius",-5)
      misn.osdDestroy()
      var.push("heretic_misn_tracker",misn_tracker)
      misn.finish(true)
   end
end

function out_sys_failure() --jumping pre-emptively is a bad thing.
   if system.cur() ~= homesys then
      tk.msg(misn_title,out_sys_failure_msg)
      misn.osdDestroy()
      misn.finish(false)
   end
end

function abort()
   misn.osdDestroy()
   misn.finish(false)
end
   
