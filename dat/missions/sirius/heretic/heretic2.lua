--[[misn title - the patrol]]
--[[in this mission, the player will be guarding the "high command" of the
   nasin, the wringer/suna. house sirius is sending in recon parties.
   the players job is to take out any and all sirius in the system.]]

include "dat/scripts/numstring.lua"

bmsg = {}
--beginning messages
bmsg[1] = _([[You walk up to an intimidating man dressed smartly in cool, dark black business attire. He has a large smile spread across his face. 
    "Ahh, so you're the %s everyone has been talking about. Naught but a glorified delivery boy if you ask me. Still, if you wish to help us out and prove yourself as more than a pirate, I'd be more than happy to oblige." He grins cooly, expecting an answer.]])
bmsg[2] = _([[Draga snorts impatiently. "Well, do you take the mission or what?"]])
choice = {}
choice[1] = _("Tell me more about the Nasin.")
choice[2] = _("What's the job?")
choice[3] = _("I'm in. Where do I sign up?")
choice[4] = _("Sounds risky. Give me some time.")
chooser = {}
chooser[1] = _([[He looks at you, betraying a little emotion. "The Nasin are a pure piece of glass. When light shines through glass, the light is only as pure as the glass itself. If the glass is dirty, then the light is distorted, and doesn't come through correctly. If the glass is mishappen or broken, the light may not filter through at all. We, the Nasin, are the purest glass there is, and House Sirius has become corrupt. We exist to see its downfall."]])
chooser[2] = _([[Draga motions you in closer. "We have reason to believe that %s is about to be attacked. We are expecting Sirius to send in recon elements any hectosecond now. We want you to handle that, as you see fit. Keep them away from the station. Better yet, kill them. We can pay you %s. We do ask that you stay in-system and off-planet until the mission is complete, otherwise you'll be considered AWOL, which means you're fired."]])
chooser[3] = _([[Draga looks triumphant, but only for an instant. "Great. You should get going, we are expecting them at any second. Good luck, and godspeed."]])
chooser[4] = _([[You brace yourself, as Draga appears ready to attack. He waves his arms about in obvious anger. "Great! I knew you were a waste of time. Well, if you decide to outgrow your diapers, I'll be right here waiting for you."
   You walk away insulted, but strangely curious.]])
--message at the end
emsg_1 = _([[You land, having destroyed the small recon force. Draga is in the hangar, waiting for you.
   "Good job on proving yourself more than a delivery boy! That wasn't so bad, was it? Here's your payment, meet me in the bar soon."]])
--mission osd
osd = {}
osd[1] = _("Destroy the Sirius fighter element.")
osd[2] = _("Element destroyed. Land on %s.")
--random odds and ends
misn_title = _("The Patrol")
npc_name = _("Draga")
bar_desc = _("An imposing man leans against the bar easily, looking right at you.")
not_finished = _("Draga seems to swoop in out of nowhere as soon as you land. \"You are not finished! Get back up there quickly!\"")
chronic_failure = _([[Draga swoops in. His nostrils are flaring, and he is obviously annoyed.
   "Apparently you have better things to do. Get out of here. I don't want to see your face anymore."
   You consider yourself fired.]])
doom_clock_msg = _([[A scratchy voice jumps in on your comms priority channel. 
   "You jumped out of the system! We are being scanned by the enemy! We need you back to take care of this situation now, or you are AWOL and are not getting paid!!" 
   The voice and the scratch cuts out.]])
out_sys_failure_msg = _([[Your comm station flares up with a scratchy, obviously-from-far-away noise. A voice is heard through it.
   "%s! We told you we needed you to stay in system! Apparently you have more important things to do. So get lost, kid! We'll take care of ourselves." The static cuts out, and you consider yourself fired.]])
misn_desc = _("Destroy the Sirius recon element that flew into %s. WARNING: DO NOT JUMP OUT-SYSTEM OR LAND ON THE PLANET PREMATURELY.")
misn_reward = _("%s credits")

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
   reward = math.floor((100000+(math.random(5,8)*2000)*(nasin_rep^1.315))*.01+.5)/.01
   chronic = 0
   finished = 0
   takeoff_counter = 0
   draga_convo = 0
   deathcount = 0
   --set the mission stuff
   misn.setTitle(misn_title)
   misn.setReward(misn_reward:format(numstring(reward)))
   misn.setNPC(npc_name,"neutral/thief2")
   misn.setDesc(bar_desc)

   -- Format OSD
   osd[2] = osd[2]:format( homeasset:name() )

   misn_desc = misn_desc:format(homesys:name())
end

function accept()
   -- Only referenced in this function.
   chooser[2] = chooser[2]:format(homeasset:name(), numstring(reward))

   while true do --this is a slightly more complex convo than a yes no. Used break statements as the easiest way to control convo.

      msg = bms
      if first_talk == nil then --used the first talk one only the first time through. This way, the npc convo feels more natural.
         msg = bmsg[1]:format( player.name() )
         first_talk = 1
      else
         msg = bmsg[2]
      end
      draga_convo = tk.choice(misn_title, msg,
            choice[1], choice[2], choice[3], choice[4])
         draga_convo = tk.choice(misn_title,bmsg[2],choice[1],choice[2],choice[3],choice[4])

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
      tk.msg(misn_title, out_sys_failure_msg:format( player.name() ))
      misn.osdDestroy()
      misn.finish(false)
   end
end

function abort()
   misn.osdDestroy()
   misn.finish(false)
end
   
