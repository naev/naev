--[[

   This is the third mission of the Shark's teeth campaign. The player has to take illegal holophone recordings in his ship.

   Stages :
   0) Way to Sirius world
   1) Way to Darkshed

   TODO: I didn't test the case when the player tries to do the mission with a freighter, and the case when the player's class is unknown

--]]

include "numstring.lua"

title = {}
text = {}
osd_msg = {}
npc_desc = {}
bar_desc = {}

title[1] = _("Nexus Shipyards needs you (again)")
text[1] = _([[You sit at Smith's table and ask him if he has a job for you. "Of course," he answers. "But this time, it's... well...
    "Listen, I need to explain some background. As you know, Nexus designs are used far and wide in smaller militaries. The Empire is definitely our biggest customer, but the Frontier also notably makes heavy use of our Lancelot design, as do many independent systems. Still, competition is stiff; House Dvaered's Vendetta design, for instance, is quite popular with the FLF, ironically enough.
    "But matters just got a little worse for us: it seems that House Sirius is looking to get in on the shipbuilding business as well, and the Frontier are prime targets. If they succeed, the Lancelot design could be completely pushed out of Frontier space, and we would be crushed in that market between House Dvaered and House Sirius. Sure, the FLF would still be using a few Pacifiers, but it would be a token business at best, and not to mention the authorities would start associating us with terrorism.
    "So we've conducted a bit of espionage. We have an agent who has recorded some hopefully revealing conversations between a House Sirius sales manager and representatives of the Frontier. All we need you to do is meet with the agent, get the recordings, and bring them back to me on %s in the %s system." You raise an eyebrow.
    "It's not exactly legal. That being said, you're just doing the delivery, so you almost certainly won't be implicated. What do you say? Is this something you can do?"]])

refusetitle = _("Sorry, not interested")
refusetext = _([["Ok, sorry to bother you."]])

title[2] = _("The job")
text[2] = _([["I'm glad to hear it. Go meet our agent on %s in the %s system. Oh, yes, and I suppose I should mention that I'm known as 'James Neptune' to the agent. Good luck!"]])

title[3] = _("Good job")
text[3] = _([[The Nexus employee greets you as you reach the ground. "Excellent! I will just need to spend a few hectoseconds analyzing these recordings. See if you can find me in the bar soon; I might have another job for you.]])

title[4] = _("Time to go back to Alteris")
text[4] = _([[You approach the agent and obtain the package without issue. Before you leave, he suggests that you stay vigilant. "They might come after you," he says.]])


-- Mission details
misn_title = _("Unfair Competition")
misn_reward = _("%s credits")
misn_desc = _("Nexus Shipyard is in competition with House Sirius.")

-- NPC
npc_desc[1] = _("Arnold Smith")
bar_desc[1] = _([[Arnold Smith is here. Perhaps he might have another job for you.]])
npc_desc[2] = _("Nexus's agent")
bar_desc[2] = _([[This guy matches exactly the description that was made to you.]])

-- OSD
osd_title = _("Unfair Competition")
osd_msg[1] = _("Land on %s in %s and meet the Nexus agent")
osd_msg[2] = _("Bring the recording back to %s in the %s system")

function create ()

   --Change here to change the planets and the systems
   mispla,missys = planet.getLandable(faction.get("Sirius"))

   while mispla:services()["bar"] == false do  --It must be a bar on this Planet
      mispla,missys = planet.getLandable(faction.get("Sirius"))
   end

   pplname = "Darkshed"
   psyname = "Alteris"
   paysys = system.get(psyname)
   paypla = planet.get(pplname)

   if not misn.claim(missys) then
      misn.finish(false)
   end

   misn.setNPC(npc_desc[1], "neutral/male1")
   misn.setDesc(bar_desc[1])
end

function accept()

   stage = 0
   reward = 750000
   proba = 0.3  --the chances you have to get an ambush

   if tk.yesno(title[1], text[1]:format(pplname, psyname)) then
      misn.accept()
      tk.msg(title[2], text[2]:format(mispla:name(),missys:name()))

      osd_msg[1] = osd_msg[1]:format(mispla:name(),missys:name())
      osd_msg[2] = osd_msg[2]:format(pplname, psyname)

      misn.setTitle(misn_title)
      misn.setReward(misn_reward:format(numstring(reward)))
      misn.setDesc(misn_desc)
      osd = misn.osdCreate(osd_title, osd_msg)
      misn.osdActive(1)

      marker = misn.markerAdd(missys, "low")

      landhook = hook.land("land")
      enterhook = hook.enter("enter")
      else
      tk.msg(refusetitle, refusetext)
      misn.finish(false)
   end
end

function land()
   --The player is landing on the mission planet to get the box
   if stage == 0 and planet.cur() == mispla then
      agent = misn.npcAdd("beginrun", npc_desc[2], "neutral/scientist", bar_desc[2])
   end

   --Job is done
   if stage == 1 and planet.cur() == paypla then
      if misn.cargoRm(records) then
         tk.msg(title[3], text[3])
         player.pay(reward)
         misn.osdDestroy(osd)
         hook.rm(enterhook)
         hook.rm(landhook)
         misn.finish(true)
      end
   end
end

function enter()
   -- Ambush !
   if stage == 1 and rnd.rnd() < proba then
      ambush()
      proba = proba - 0.2
      elseif stage == 1 then
      --the probality of an ambush goes up when you cross a system without meeting any ennemy
      proba = proba + 0.1
   end
end

function beginrun()
   tk.msg(title[4], text[4])
   records = misn.cargoAdd("Box", 0)  --Adding the cargo
   stage = 1
   misn.osdActive(2)
   misn.markerRm(marker)
   marker2 = misn.markerAdd(paysys, "low")

   --remove the spy
   misn.npcRm(agent)
end

function ambush()
   --Looking at the player ship's class in order to spawn the most dangerous ennemy to him
   playerclass = player.pilot():ship():class()
   badguys = {}

   if playerclass == "Scout" or playerclass == "Fighter" or playerclass == "Drone" or playerclass == "Heavy Drone" or playerclass == "Luxury Yacht" or playerclass == "Yacht" or palyerclass == "Courier" then

      if rnd.rnd() < 0.7 then
         interceptors()
         else
         hvy_intercept()
      end

      elseif playerclass == "Bomber" then

      local rand = rnd.rnd()
      if rand < 0.5 then
         hvy_intercept()
         elseif  rand < 0.8 then
         interceptors()
         else
         corvette()
      end

      elseif playerclass == "Freighter" then  --what a strange idea to use a Mule in this situation...

      if rnd.rnd() < 0.6 then
         corvette()
         else
         cruiser()
      end

      elseif playerclass == "Corvette" or playerclass == "Destroyer" then

      local rand = rnd.rnd()
      if rand < 0.6 then
         cruiser()
         elseif rand < 0.8 then
         corvette()
         else
         hvy_intercept()
      end

      elseif playerclass == "Cruiser" or playerclass == "Armoured Transport" or playerclass == "Carrier" then

      if rnd.rnd() < 0.7 then
         bombers()
         else
         hvy_intercept()
      end

      else     --The fact you don't have a ship class in the list doesn't means you're safe !
      littleofall()
   end
   --and a Llama for variety :
   if rnd.rnd() < 0.5 then
      add_llama()
   end
end

function interceptors()
   --spawning high speed Hyenas
   number = {1,2,3,4}
   for i in ipairs(number) do
      badguys[i] = pilot.addRaw( "Hyena","mercenary", nil, "Mercenary" )
      badguys[i]:setHostile()

      badguys[i]:rename(_("Mercenary"))
      --Their outfits must be quite good
      badguys[i]:rmOutfit("all")
      badguys[i]:rmOutfit("cores")

      badguys[i]:addOutfit("Unicorp D-2 Light Plating")
      badguys[i]:addOutfit("Unicorp PT-100 Core System")
      badguys[i]:addOutfit("Tricon Zephyr Engine")

      badguys[i]:addOutfit("Laser Cannon MK2",2)
      badguys[i]:addOutfit("Unicorp Fury Launcher")
      badguys[i]:addOutfit("Improved Stabilizer") -- Just try to avoid fight with these fellas

      badguys[i]:setHealth(100,100)
      badguys[i]:setEnergy(100)
   end
end

function hvy_intercept()
   --spawning Lancelots
   number = {1,2,3,4}
   for i in ipairs(number) do
      badguys[i] = pilot.addRaw( "Lancelot","mercenary", nil, "Mercenary" )
      badguys[i]:setHostile()

      badguys[i]:rename(_("Mercenary"))
      --Their outfits must be quite good
      badguys[i]:rmOutfit("all")
      badguys[i]:rmOutfit("cores")

      badguys[i]:addOutfit("Unicorp D-4 Light Plating")
      badguys[i]:addOutfit("Unicorp PT-200 Core System")
      badguys[i]:addOutfit("Tricon Zephyr II Engine")

      badguys[i]:addOutfit("Mass Driver MK1")
      badguys[i]:addOutfit("Shredder",2)
      badguys[i]:addOutfit("Ripper Cannon")
      badguys[i]:addOutfit("Shield Capacitor",2)

      badguys[i]:setHealth(100,100)
      badguys[i]:setEnergy(100)
   end
end

function corvette()
   --spawning Admonishers
   number = {1,2}
   for i in ipairs(number) do
      badguys[i] = pilot.addRaw( "Admonisher","mercenary", nil, "Mercenary" )
      badguys[i]:setHostile()
      badguys[i]:rename(_("Mercenary"))

      badguys[i]:rmOutfit("all")
      badguys[i]:rmOutfit("cores")

      badguys[i]:addOutfit("Unicorp D-8 Medium Plating")
      badguys[i]:addOutfit("Unicorp PT-500 Core System")
      badguys[i]:addOutfit("Tricon Cyclone Engine")

      badguys[i]:addOutfit("Razor Turret MK3",2)
      badguys[i]:addOutfit("Unicorp Headhunter Launcher",2)

      badguys[i]:setHealth(100,100)
      badguys[i]:setEnergy(100)
   end
end

function cruiser()
   --spawning a Krestel with massive missile weaponry
   badguy = pilot.addRaw( "Kestrel","mercenary", nil, "Mercenary" )
   badguy:setHostile()
   badguy:rename(_("Mercenary"))

   badguy:rmOutfit("all")
   badguy:rmOutfit("cores")

   badguy:addOutfit("Unicorp D-16 Heavy Plating")
   badguy:addOutfit("Unicorp PT-900 Core System")
   badguy:addOutfit("Krain Remige Engine")

   badguy:addOutfit("Heavy Ripper Turret",2)
   badguy:addOutfit("Unicorp Headhunter Launcher",2)
   badguy:addOutfit("Enygma Systems Spearhead Launcher",2)

   badguy:addOutfit("Large Shield Booster",2)
   badguy:addOutfit("Improved Stabilizer",4)
   badguy:setHealth(100,100)
   badguy:setEnergy(100)

end

function bombers()
   --spawning Ancestors
   number = {1,2,3}
   for i in ipairs(number) do
      badguys[i] = pilot.addRaw( "Ancestor","mercenary", nil, "Mercenary" )
      badguys[i]:setHostile()
      badguys[i]:rename(_("Mercenary"))

      badguys[i]:rmOutfit("all")
      badguys[i]:rmOutfit("cores")

      badguys[i]:addOutfit("S&K Ultralight Combat Plating")
      badguys[i]:addOutfit("Milspec Prometheus 2203 Core System")
      badguys[i]:addOutfit("Tricon Zephyr II Engine")

      badguys[i]:addOutfit("Unicorp Caesar IV Launcher",2)
      badguys[i]:addOutfit("Neutron Disruptor")
      badguys[i]:addOutfit("Vulcan Gun")

      badguys[i]:addOutfit("Small Shield Booster",2)
      badguys[i]:addOutfit("Shield Capacitor",2)

      badguys[i]:setHealth(100,100)
      badguys[i]:setEnergy(100)
   end
end

function add_llama()
   --adding an useless Llama
   useless = pilot.addRaw( "Llama","mercenary", nil, "Mercenary" )
   useless:setHostile()
   useless:rename(_("Amateur Mercenary"))

   useless:rmOutfit("all")
   useless:addOutfit("Laser Cannon MK1",2)

   useless:setHealth(100,100)
   useless:setEnergy(100)
end

function littleofall()
   --spawning random ennemies
   if rnd.rnd() < 0.5 then
      interceptors()
      else
      hvy_intercept()
   end
end

function abort()
   if stage == 1 then
      misn.cargoRm(records)
   end
   misn.finish(false)
end
