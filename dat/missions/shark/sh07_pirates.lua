--[[
   
   This is the climax mission of the Shark's teeth campaign. The player has to kill 4 pirates.
   
   Stages :
   0) There are pirates to kill
   1) Way to Alteris
   
--]]

--Needed scripts
include "dat/scripts/pilot/pirate.lua"
include "dat/scripts/numstring.lua"
include "dat/scripts/jumpdist.lua"

lang = naev.lang()
if lang == "es" then
   else -- default english
   title = {}
   text = {}
   osd_msg = {}
   npc_desc = {}
   bar_desc = {}
   
   title[1] = "The mission"
   text[1] = [["Hello again," says Smith. "As you know, I've agreed with the FLF on a contract that will make it possible for Nexus to sell them hundreds of "Shark" light fighters. This income will easily make up for the losses due to the partial replacement of the "Shark" by drones in the Imperial fleet.
   Of course, this transaction must remain a secret. Even the head of the FLF thinks that we are working for an outlaw organization instead of a multi-stellar company. We are going to create a sales subsidiary on a pirate world so nobody will find out who really is behind this deal.
   But, as you know, the pirate worlds are the territory of the "Skulls & Bones", the outlaw shipyard that makes copies of legal ships and sells them to pirates. If they discover our project, they will do everything to destroy us. That's why we need to make the first move: Skulls & Bones use four major hitmen, who are also pirates. I want you to kill these four pirates in order to show the Skulls & Bones that they can't win against us. I think it's the safest way to do what we have to do. Of course, there is a bounty on each of their heads.
   Are you in?"]]
   
   refusetitle = "Sorry, not interested"
   refusetext = [["Ok, too bad, you are the only one I can trust for this job. Don't hesitate to come back if you change your mind."]]
   
   title[2] = "Very good"
   text[2] = [["So, here are the details: %s is around %s, flying his Gawain: he is taking an undercover holiday, spending all the money he has stolen from traders. %s is in %s and %s is around %s with his Kestrel. Be careful, they have escorts. %s is in %s with his fearsome stolen Goddard and his escort.
   Come back when you have finished; I will give you your bounties."]]
   
   title[3] = "Well done!"
   text[3] = [[This one will never get in our way again.]]
   
   title[4] = "Mission accomplished"
   text[4] = "You have killed the four pirates: Adam Smith is probably waiting for you in %s with lots of money."
   
   title[5] = "That was impressive"
   text[5] = [[As you land, Smith is already there. "Thanks to your actions I managed to create the sales subsidiary and I think nobody will prevent us from selling Sharks anymore. It was very nice working with you. Here is your bounty. Good luck in the future."]]

   title[6] = "Target is gone!"
   text[6] = [[It seems the pirate went away. Don't worry: I bet you that if you come back in some time, he will still be there."]]

   -- Mission details
   misn_title = "The Last Detail"
   misn_reward = "%s credits"
   misn_desc = "Nexus Shipyard asked you to kill four pirates"
   
   -- NPC
   npc_desc[1] = "Arnold Smith"
   bar_desc[1] = [[Smith probably has a mission for you that involves "fixing a detail", as he says.]]
   
   -- OSD
   osd_title = "The Last Detail"
   osd_msg[1] = "Kill the four pirates"
   osd_msg[2] = "Report back to %s in %s"
end

function create ()
   
   --Will now pick systems between min and max jumps in distance
   local min = 3
   local max = 7
   local systems = getsysatdistance(system.cur(), min, max)
   if #systems == 0 then
      local systems = getsysatdistance(system.cur(), 1, 15)
      if #systems == 0 then
         osd_title = "Houston, we have a problem!"
         gawsys = system.get("Alteris")
         kersys1 = system.get("Alteris")
         kersys2 = system.get("Alteris")
         godsys = system.get("Alteris")
      end
   end
   local index = rnd.rnd(1, #systems/4) --This avoids picking the same
   gawsys = systems[index]
   local index = rnd.rnd(index, #systems/3) --system twice, which would
   kersys1 = systems[index]
   local index = rnd.rnd(index, #systems/2) --make the missions rather
   kersys2 = systems[index]
   local index = rnd.rnd(index, #systems) --harder!
   godsys = systems[index]

   pplname = "Darkshed"
   psyname = "Alteris"
   paysys = system.get(psyname)
   paypla = planet.get(pplname)
   
   if not misn.claim(gawsys) and misn.claim(kersys1) and misn.claim(kersys2) and misn.claim(godsys) then
      misn.finish(false)
   end
   
   misn.setNPC(npc_desc[1], "neutral/male1")
   misn.setDesc(bar_desc[1])
end

function accept()
   
   reward = 4000000
   stage = 0
   
   --Initialization of dead pirate memory
   gawdead = false
   kerdead1 = false
   kerdead2 = false
   goddead = false
   
   --set the names of the pirates (and make sure they aren't duplicates)
   gawname = pirate_name()
   kername1 = pirate_name() .. " III"
   kername2 = pirate_name() .. " Jr."
   godname = pirate_name() .. " II"
   
   if tk.yesno(title[1], text[1]) then
      misn.accept()
      tk.msg(title[2], text[2]:format(gawname,gawsys:name(),kername1,kersys1:name(),kername2,kersys2:name(),godname,godsys:name()))
      
      osd_msg[2] = osd_msg[2]:format(pplname,psyname)
      
      misn.setTitle(misn_title)
      misn.setReward(misn_reward:format(numstring(reward)))
      misn.setDesc(misn_desc)
      osd = misn.osdCreate(osd_title, osd_msg)
      misn.osdActive(1)
      
      gawmarker = misn.markerAdd(gawsys, "low")
      kermarker1 = misn.markerAdd(kersys1, "high")
      kermarker2 = misn.markerAdd(kersys2, "high")
      godmarker = misn.markerAdd(godsys, "high")
      
      enterhook = hook.enter("enter")
      landhook = hook.land("land")
      
      else
      tk.msg(refusetitle, refusetext)
      misn.finish(false)
   end
end

function land()   
   --Job is done
   if stage == 1 and planet.cur() == planet.get("Darkshed") then
      tk.msg(title[5], text[5])
      player.pay(reward)
      misn.osdDestroy(osd)
      hook.rm(enterhook)
      hook.rm(landhook)
      misn.finish(true)
   end
end

function enter()
   --Arrived in system
   if system.cur() == gawsys and gawdead == false then  --The Gawain

      -- Choose a random point in the system for him to stay around
      sysrad = rnd.rnd() * system.cur():radius()
      angle = rnd.rnd() * 360
      pos = vec2.newP(sysrad, angle)

      baddie = pilot.addRaw( "Gawain","dummy", nil, "Thugs" )
      baddie:rename(gawname)
      baddie:setHostile()
      baddie:setHilight()
      baddie:control()
      baddie:goto(pos)
      
      --The pirate becomes nice defensive outfits
      baddie:rmOutfit("all")
      baddie:rmOutfit("cores")
      
      baddie:addOutfit("S&K Ultralight Stealth Plating")
      baddie:addOutfit("Milspec Aegis 2201 Core System")
      baddie:addOutfit("Tricon Zephyr Engine")
      baddie:setHealth(100,100)
      baddie:setEnergy(100)
      
      baddie:addOutfit("Shield Capacitor",2)
      baddie:addOutfit("Small Shield Booster")
      baddie:addOutfit("Milspec Scrambler")
      
      baddie:addOutfit("Laser Cannon MK3",2)
      
      hook.pilot(baddie, "idle", "idle", pos)
      hook.pilot(baddie, "attacked", "attacked")
      hook.pilot(baddie, "death", "gawain_dead")
      hook.pilot(baddie, "jump", "generic_jumped")
      
      elseif system.cur() == kersys1 and kerdead1 == false then  --The Kestrel
      pilot.clear()
      pilot.toggleSpawn(false)
      
      baddie = pilot.add( "Pirate Kestrel", nil, vec2.new(0,0))[1]
      ancestor = pilot.add( "Pirate Ancestor", nil, vec2.new(100,0))[1]
      hyena = pilot.add( "Pirate Hyena", nil, vec2.new(0,100))[1]
      
      baddie:rename(kername1)
      baddie:setHilight()
      baddie:setHostile()
      
      hook.pilot( baddie, "death", "kestrel_dead1")
      hook.pilot(baddie, "jump", "generic_jumped")
      
      elseif system.cur() == kersys2 and kerdead2 == false then  --The Kestrel
      pilot.clear()
      pilot.toggleSpawn(false)
      
      baddie = pilot.add( "Pirate Kestrel", nil, vec2.new(0,0))[1]
      ancestor = pilot.add( "Pirate Ancestor", nil, vec2.new(100,0))[1]
      shark = pilot.add( "Pirate Shark", nil, vec2.new(0,100))[1]
      hyena = pilot.add( "Pirate Hyena", nil, vec2.new(100,100))[1]
      
      baddie:rename(kername2)
      baddie:setHilight()
      baddie:setHostile()
      
      hook.pilot( baddie, "death", "kestrel_dead2")
      hook.pilot(baddie, "jump", "generic_jumped")
      
      elseif system.cur() == godsys and goddead == false then  --The Goddard
      pilot.clear()
      pilot.toggleSpawn(false)
      
      baddie = pilot.add( "Goddard Goddard", nil, vec2.new(0,0))[1] --Faction's ships come with upgraded weaponry
      baddie:setFaction("Pirate")
      baddie:changeAI( "pirate" )
      
      ancestor = pilot.add( "Pirate Ancestor", nil, vec2.new(100,0))[1]
      hyena = pilot.add( "Pirate Hyena", nil, vec2.new(0,100))[1]
      
      baddie:rename(godname)
      baddie:setHilight()
      baddie:setHostile()
      
      hook.pilot( baddie, "death", "goddard_dead")
      hook.pilot(baddie, "jump", "generic_jumped")
      
   end
   
end

function idle(pilot,pos)  --the Gawain is flying around a random point
   baddie:goto(pos + vec2.new( 800,  800), false, false)
   baddie:goto(pos + vec2.new(-800,  800), false, false)
   baddie:goto(pos + vec2.new(-800, -800), false, false)
   baddie:goto(pos + vec2.new( 800, -800), false, false)
end

function attacked()  --the Gawain is going away
   if baddie:exists() then
      baddie:taskClear() 
      baddie:runaway(player.pilot(), true)
   end
end

function gawain_dead()
   misn.markerRm(gawmarker)
   gawdead = true
   hook.timer(3000,"generic_dead")
end

function kestrel_dead1()
   misn.markerRm(kermarker1)
   kerdead1 = true
   hook.timer(3000,"generic_dead")
end

function kestrel_dead2()
   misn.markerRm(kermarker2)
   kerdead2 = true
   hook.timer(3000,"generic_dead")
end

function goddard_dead()
   misn.markerRm(godmarker)
   goddead = true
   hook.timer(3000,"generic_dead")
end

function generic_dead()
   player.msg(text[3])
   
   --Are there still other pirates to kill ?
   if gawdead == true and kerdead1 == true and kerdead2 == true and goddead == true then
      tk.msg(title[4], text[4]:format(paysys:name()))
      stage = 1
      misn.osdActive(2)
      marker2 = misn.markerAdd(paysys, "low")
   end
end

function gnenric_jumped()
   tk.msg(title[6], text[6])
end
