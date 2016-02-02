--[[
--
-- MISSION: The one with the Visit
-- DESCRIPTION: Dr. Geller asks you to disable a soromid ship to retrieve some technology
-- that he needs for his prototype. Za'lek science can obviously not fall behind soromid.
-- Obviously soromid will not like this and will try to attack the player.
-- 
-- Difficulty: Easy to Medium?
--
-- Author: fart but based on Mission Ideas in wiki: wiki.naev.org/wiki/Mission_Ideas
--]]

-- Localization, choosing a language if naev is translated for non-english-speaking locales.
lang = naev.lang()
if lang == "es" then
else -- Default to English
-- mission variables
   t_sys = {}
   t_pla = {}
   t_pla[1] = "Gastan"
   t_sys[1] = "Seiben"
   t_sys[2] = "Shikima"
   -- player makes maximum 50k, minimum 30k
   reward = rnd.rnd(3,4)*10000+rnd.rnd(10)*1000 
   shpnm = "Tokera"
   -- Mission details
   title = {}
   text = {}
   osd_msg = {}
   misn_title = "The one with the Visit"
   misn_reward = "A the gratitude of science and a bit of compensation"
   misn_desc = "You've been hired by Dr. Geller to retrieve technology he urgently needs to build his prototype."
   bar_desc = "You see Dr Geller and he is waving you over. Apparently he has another job for you."


   title[1] = [[In the bar]]
   text[1]  = [["Ah there you are! I've got a job for you. I am so close to revolutionizing science! I will become a dinosaur of science! The T-Rex of progress! Well, this job involves some acquisition of technology from the Soromid. They did not want to give it to me and so for science you will have to get it for me. Are you up for it?]]
   text[2] = [["Excellent. From what I have been told it looks like this." He says gesticulating around with his hands to no merit. "You will recognize it, it is in a box that should be kept separately from the remaining stuff and labelled top secret! Not secret enough now, I suppose! Ah you might need this!" and he hands you a handheld device.
"Oh and the ship is called the %s"
You realize that you are supposed to steal something top secret. Quirky people those Za'leks. With the coordinates, the signature of the target ship and the handheld, which you hope helps you detect the box, you get yourself on the way.]]
-- msgs by soromid forces
   title[2] = [[Intercom]]
   text[3] = [["You have committed and act of aggression against the House Soromid. Stand down or suffer the consequences."]]
   title[3] = [[In the ship]]
   text[4] = [[You make your way through the ship, the crew is obviously shaken and not very prepared for fighting. By wielding your guns you can convince them to not cause any trouble, so you can see how they despise you with their looks. Clearly you have not made any friends here today, but who wants to be friends with those mutants anyway, you justify to yourself. You search through the ship and the handheld in your pocket starts beeping. 
It appears to be some sort of detector allowing you to home in on the technology. You manage to locate a box on a table in one of the crew chambers. Apparently nobody expected somebody to be nuts enough to try to do what you are doing. You grab the box and head back to your ship.
Well this was the easy part, you think. Now off to the escape.]]

   text[5] = [["Have you got it?" asks Dr Geller. "Ah, marvelous! Do you know what this is? This is a quantum sharpener. It's like a quantum eraser, but it does not erase but sharpen. This is exactly what I needed. I think with this I should be able to finish my Prototype. 
"Oh, and this is for you!" and he tosses you a payslip before walking off, smiling.]]
   -- if the player kills the ship before getting the tech
   title[4] = [[What have you done?]]
   text[6] = [[The ship explodes before your eyes and you realize that you will never be able to get the secret tech now. You have failed Dr Geller! And even worse: you failed science!]]
   OSDtitle = "The one with the Visit"
   osd_msg = {}
   osd_msg[1] = "Go to the %s system and find the %s."
   osd_msg[2] = "Board the %s and retrieve the secret technology."
   osd_msg[3] = "Return to %s in the %s system."

  -- refuestext 
   refusetitle = "No Science Today"
   refusetext = "But I really thought you were into science..."
end



function create ()
   -- Spaceport bar stuff
   misn.setNPC( "Dr. Geller",  "zalek_scientist_placeholder")
   misn.setDesc( bar_desc )
end
function accept()
   -- Mission details:
   if not tk.yesno( title[1], text[1] ) then
      tk.msg(refusetitle, refusetext)
      misn.finish()
   end
   tk.msg( title[1], text[2]:format(shpnm) )
   misn.accept()
   misn.osdCreate(misn_title, {osd_msg[1]:format(t_sys[2],shpnm),osd_msg[2]:format(shpnm),osd_msg[3]:format(t_pla[1],t_sys[1])})
   misn.setDesc(misn_desc)
   misn.setTitle(misn_title)
   misn.osdActive(1)
   misn_mark = misn.markerAdd( system.get(t_sys[2]), "high" )
   targetalive = true
   hook.enter("sys_enter")
end


function sys_enter ()
   if system.cur() == system.get(t_sys[2]) and targetalive then
      dist = rnd.rnd() * system.cur():radius() *1/2
      angle = rnd.rnd() * 2 * math.pi
      location = vec2.new(dist * math.cos(angle), dist * math.sin(angle)) -- Randomly spawn the Ship in the system
      target = pilot.add("Soromid Odium", nil, location)[1]
      target:control()
      target:rename(shpnm)
      target:setFaction("Soromid")
      target:memory("aggressive", true)
      target:setHilight(true)
      target:setVisplayer(true)
      hidle = hook.pilot(target, "idle", "targetIdle")
      hexp = hook.pilot(target, "exploded", "targetExploded")
      hboard = hook.pilot(target, "board", "targetBoard")
      hnoharm = hook.pilot(target, "attacked", "tar_attacked")
      targetIdle()
   end
end

function targetIdle()
   if not pilot.exists(target) then -- Tear down now-useless hooks.
      hook.rm(hidle)
      return
   end
   if target:hostile() then
      hook.rm(hidle)
      target:control(false)
      return
   end
   location = target:pos()
   dist = 750
   angle = rnd.rnd() * 2 * math.pi
   newlocation = vec2.new(dist * math.cos(angle), dist * math.sin(angle)) -- New location is 750px away in a random direction
   target:taskClear()
   target:goto(location + newlocation, false, false)
   hook.timer(5000, "targetIdle")
end

function targetExploded()
   hook.timer( 2000, "targetDeath" )
end

function targetDeath()
   sor= faction.get("Soromid")
   hook.rm(hexp)
   if boarded then
      sor.modPlayer(-5)
      return
   end
   tk.msg(title[4],text[6])
   sor.modPlayer(-5)
   misn.finish(false)
end

function targetBoard()
   player.unboard()
   tk.msg(title[2], text[4])
   target:setHilight(false)
   target:setVisplayer(false)
   cargoID = misn.cargoAdd("Secret Technology",0)
   misn.osdActive(3)
   misn.markerRm(misn_mark)
   misn_mark = misn.markerAdd( system.get(t_sys[1]), "high" )
   hland = hook.land("land")
   boarded = true
   hook.rm(hboard)
end

function tar_attacked ()
   tk.msg(title[2], text[3])
   target:setHostile(true)
   hook.rm(hnoharm)
end

function land()
   if planet.cur() == planet.get(t_pla[1]) then
      tk.msg(title[1], text[5])
      hook.rm(hland)
      misn.markerRm(misn_mark)
      player.pay(reward) -- 30K
      misn.finish(true)
   end
end

function abort ()
   if target then
      target:setHilight(false)
      target:setVisplayer(false)
   end
   misn.finish(false)
end
