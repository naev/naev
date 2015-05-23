--[[
   
   This is the third mission of the Shark's teeth campaign. The player has to take illegal holophone recordings in his ship.
   
   Stages :
   0) Way to Sirius world
   1) Way to Darkshed
	
	TODO: I didn't test the case when the player tries to do the mission with a freighter, and the case when the player's class is unknown
	
--]]

include "numstring.lua"

lang = naev.lang()
if lang == "es" then
   else -- default english
   title = {}
   text = {}
   osd_msg = {}
   npc_desc = {}
   bar_desc = {}
   
   title[1] = "Nexus Shipyards needs you (again)"
   text[1] = [[You sit at Smith's table and ask him if he has a job for you. "Of course I have," he answers. "But this time, it's a quite special job." As you start to wonder what could be more special than the last mission, he goes further: "It implies to take part in a... quite not so legal action...
   Listen, I'll explain you the situation: the Empire, who was the first customer for the Shark, tend to use more and more drones from Robosys in its fleet, and less and less Sharks. That means for us that we will lose billions of credits if we can't find a big customer. Ingot bought 20 Sharks, but now, we need some major faction to use our fighter.
   As you surely know, the Frontier ships are mostly obsolete and old. We have clues that let us think that the Frontier is looking for a modern light fighter to equip it's pilots. The problem is that we suspect that a major imperial house is already in contact with frontier officials in order to sell them their light fighter.
   We need you to transport a... parcel on your ship, and bring it to me on %s in %s. Are you in?"]]
	
   refusetitle = "Sorry, not interested"
   refusetext = [["Ok, so never mind." Smith says. "See you again."]]
   
   title[2] = "The job"
   text[2] = [["Very good, so, go to %s in %s. There, our agent will be waiting for you and will load the special parcel in your ship. Listen, I trust you enough to tell you that it is a recording of a holophone conversation between a member of the Frontier council and a sales manager of house Sirius.
   Oh, yes, and, if you has to say my name to our contact, he knows me as James Neptune. So, good luck out there."]]
	
   title[3] = "Good job"
   text[3] = [[The Nexus employee greets you as you reach the ground and ask you if everything went well. He takes the parcel and gives you your pay. "We will analyze this, meet me in the bar, I will be there if I have an other job for you."]]
   
   title[4] = "Time to go back to Alteris"
   text[4] = [[You approach the agent, wondering why Nexus's henchmen always manage to look honest. It seems he knew who was supposed to pick his parcel. After asking the name of your contact, he gives you a little battery-looking tool. "Everything is in there," he just says. "By the way, stay vigilant, someone may be on our tracks."]]
   
	
   -- Mission details
   misn_title = "Unfair Competition"
   misn_reward = "%s credits"
   misn_desc = "Nexus Shipyard is in competition with House Sirius on a major contract."
   
   -- NPC
   npc_desc[1] = "Arnold Smith"
   bar_desc[1] = [[Arnold Smith is in the place. That means he has a mission that implies tricking a customer of Nexus.]]
   npc_desc[2] = "Nexus's spy"
   bar_desc[2] = [[This guy matches exactly the description that was made to you.]]
   npc_desc[3] = "Arnold Smith"
   bar_desc[3] = [[Arnold Smith is in the place. That means he has a mission that implies tricking a customer of Nexus.]]
	
   -- OSD
   osd_title = "Unfair Competition"
   osd_msg[1] = "Land on %s in %s and meet the Nexus agent"
   osd_msg[2] = "Bring the recording back to %s in %s"
   
end

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
   reward = 50000
   proba = 0.3  --the chances you have to get an ambush
	
   if tk.yesno(title[1], text[1]:format(papla:name(), paysys:name())) then
      misn.accept()
      tk.msg(title[2], text[2]:format(mispla:name(),missys:name()))
      
      misn.setTitle(misn_title)
      misn.setReward(misn_reward:format(numstring(reward)))
      misn.setDesc(misn_desc)

      osd_msg[1] = osd_msg[1]:format(mispla:name(),missys:name())
      osd_msg[2] = osd_msg[2]:format(papla:name(), paysys:name())

      misn.osdCreate(misn_title, osd_msg)
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
   playerclass = ship.class(pilot.ship(player.pilot()))
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
      badguys[i] = pilot.addRaw( "Hyena","mercenary", nil, "Mercenary" )[1]
      badguys[i]:setHostile()
      
      badguys[i]:rename("Mercenary")
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
      badguys[i] = pilot.addRaw( "Lancelot","mercenary", nil, "Mercenary" )[1]
      badguys[i]:setHostile()
		
      badguys[i]:rename("Mercenary")
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
      badguys[i] = pilot.addRaw( "Admonisher","mercenary", nil, "Mercenary" )[1]
      badguys[i]:setHostile()
      badguys[i]:rename("Mercenary")
      
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
   badguy = pilot.addRaw( "Kestrel","mercenary", nil, "Mercenary" )[1]
   badguy:setHostile()
   badguy:rename("Mercenary")
   
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
      badguys[i] = pilot.addRaw( "Ancestor","mercenary", nil, "Mercenary" )[1]
      badguys[i]:setHostile()
      badguys[i]:rename("Mercenary")
      
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
   useless = pilot.addRaw( "Llama","mercenary", nil, "Mercenary" )[1]
   useless:setHostile()
   useless:rename("Amateur Mercenary")
   
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
