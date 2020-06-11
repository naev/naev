--[[misn title - the patrol]]
--[[in this mission, the player will be guarding the "high command" of the
   nasin, the wringer/suna. house sirius is sending in recon parties.
   the players job is to take out any and all sirius in the system.]]

include "dat/scripts/numstring.lua"

bmsg = {}
--beginning messages
bmsg[1] = _([[You walk up to the intimidating man. He's dressed smartly in cool, dark black business attire, with a large smile spread across his face.
    "Ahh, so you're %s! Everyone has been talking about you," he says. "Allow me to introduce myself. My name is Draga, high commander of Nasin's operations. You remember us, right? People in our organization have started to take notice of your actions. Maybe it is the will of Sirichana that you come to us! If that is the case, then I have an offer you can't refuse. A chance to really prove yourself as more than a glorified courier. You see... we are expecting a Sirian patrol any hectosecond now, and we want you to... take care of it. What do you say? We could really use your help."]])
bmsg[2] = _([["Marvelous! I knew I could count on you! Don't you worry; you'll be fighting alongside some of our finest pilots. I know you can drive those Sirii off!
    "Oh, and one last thing: don't even think of bailing out on us at the last second. If you jump out or land before your mission is completed, consider yourself fired."]])
bmsg[3] = _([["Gah! I should have known you would be so spineless! Get out of my sight!"]])

--message at the end
emsg_1 = _([[You land, having defeated the small recon force, and find Draga with a smile on his face. "Great job!" he says. "I see you really are what you're made out to be and not just some overblown merchant!" He hands you a credit chip. "Thank you for your services. Meet us in the bar again sometime. We will certainly have another mission for you."]])
--mission osd
osd = {}
osd[1] = _("Destroy the Sirius patrol")
osd[2] = _("Land on %s")
--random odds and ends
misn_title = _("The Patrol")
npc_name = _("An Imposing Man")
bar_desc = _("This man leans against the bar while looking right at you.")
chronic_failure = _([[Draga's face goes red with fury when he sees you. For a moment you start to worry he might beat you into a pulp for abandoning your mission, but he moves along, fuming. You breathe a sigh of release; you may have angered Nasin, but at least you're still alive.]])
out_sys_failure_msg = _([[As you abandon your mission, you recieve a message from Draga saying that Nasin has no need for deserters. You hope you made the right decision.]])
misn_desc = _("You have been hired once again by Nasin, this time to destroy a Sirius patrol that has entered %s.")
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
   misn.setNPC(npc_name, "sirius/unique/draga")
   misn.setDesc(bar_desc)

   -- Format OSD
   osd[2] = osd[2]:format( homeasset:name() )

   misn_desc = misn_desc:format(homesys:name())
end

function accept()
   if tk.yesno( misn_title, bmsg[1]:format( player.name() ) ) then
      misn.accept()
      tk.msg( misn_title, bmsg[2] )

      misn.setDesc(misn_desc)
      misn.markerAdd(homesys,"high")
      misn.osdCreate(misn_title,osd)
      misn.osdActive(1)
      hook.takeoff("takeoff")
      hook.jumpin("out_sys_failure")
      hook.land("land")
   else
      tk.msg( misn_title, bmsg[3] )
      misn.finish( false )
   end
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
      faction.modPlayerSingle("Nasin",-20)
      misn.finish(false) 
   elseif planet.cur() == homeasset and finished == 1 then
      tk.msg(misn_title,emsg_1)
      player.pay(reward)
      misn_tracker = misn_tracker + 1
      faction.modPlayer("Nasin",7)
      var.push("heretic_misn_tracker",misn_tracker)
      misn.finish(true)
   end
end

function out_sys_failure() --jumping pre-emptively is a bad thing.
   if system.cur() ~= homesys then
      tk.msg(misn_title, out_sys_failure_msg:format( player.name() ))
      faction.modPlayerSingle("Nasin",-20)
      misn.finish(false)
   end
end
   
