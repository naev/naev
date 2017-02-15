--[[
   Mission: The one with the Runaway
   Description: A prototype drone runs away and needs to be caught by the player
   How to make this more interesting than just go to a system and shoot down the drone w ion or medusa? The ship has to have enough cargo for the drone to be transported back.
   So the player has to hail the drone bc the scientist wants to patch the software which fails and the drone causes other drones to attack.
   I was thinking of player having to use electronic warfare. But is there a way to check that?
   Difficulty: relatively easy? 

   Author: fart but based on Mission Ideas in wiki: wiki.naev.org/wiki/Mission_Ideas
--]]

include "fleethelper.lua"
include "factions/spawn/zalek/lua"
-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
-- set text variables
   title = {}
   text = {}
   osd_msg = {}
   -- system with the drone and the return to start
   t_sys = {}
   t_pla = {}
   -- Mission details
   reward = 100000
   -- amount of jumps the drone did to escape. Each jump reduces it's speed
   fled = false
   jumps = 0 
   t_sys[1] = "Xavier"
   t_pla[1] = system.get(t_sys[1]):planets()[1]
   t_sys[2] = "Seiben"
   t_pla[2] = "Gastan"
   misn_title = "The one with the Runaway"
   misn_reward = "A peek at the new prototype and some compensation for your efforts"
   misn_desc = "You've been hired by Dr. Geller to retrieve his prototype that ran away."
   bar_desc = "You see Dr.Geller going from one person to the next, apparently asking for something. However he seems to not get the answer he wants..."

   title[1] = [[In the bar]]
   title[2] = [[On the intercom]]
   title[3] = [[On your ship]]
   title[4] = [[Back on %s]]
   text[1]  = [["Hey you! Yeah you! I need your help. Ha! I even remember you! I've got another job for you, and I hope you do it as well as the ones before. You see I was finishing up my prototype. It's ingenious I tell you. It is going to be perfect. Yes, indeed." After you give him a confused look he stops for a second. "Your prototype..." you try to get him going again. "Ah, right, I told you about it! Well you see, there was a minor hiccup." You realize you don't really like the sound of that.
"It's nothing major, it is just, well, that I lost it." You chuckle. You were suspecting that for a long time. "But I would not be Dr. Geller if I had not put a tracking mechanism into it! Ingenious! I told you, didn't I? Well so I want you to catch it, bring it back, ok? You can do that, right?"]]
   text[2] = [["Excellent, let's go. You were not really thinking I would let you engage my prototype without my supervision? I will show you in which system the drone currently is. But you better be fast, this is the newest generation and I bet it can outrun most of the ships out there. And of course: This is top secret research! Nobody outside of this institution has to know about this!"
The scientist follows you around to your ship. Going on about how important this research is and pestering you to get going better sooner than later...]]
   text[3] = [["There! The tracker shows it must be here! It is right next to %s! Amazing, right? If you hail it I might be able to patch the software. That should give me control again. But you have to be close so the data transfer is as stable as possible."]]
   text[4] = [["Huh, I don't understand. This should not be happening. Hold on. I can't get access."]]
   text[5] = [["Uhm, there seems to be a glitch. Well, sort of. Uhm, if I decipher this correctly, the drone just hijacked the unused drones on %s and ordered them to attack us. I never should have tempered with that weird chip the pirates sold me!"]]
   text[6] = [["If you can disable the prototype, do it, but I'd rather prefer not to die!"]]
   text[7] = [["Excellent work, now load it up and let's get out of here!"]]
   text[8] = [["Interesting, the other drones are running away..."
"All the better."]]
   -- is that the way to go? get person to jump and try again? that sounds a bit annoying maybe to play through
   text[9] = [["The drone has disappeared from my radar! It must have jumped to the %s system. Let's find it!"]]
   text[10] = [["The scanner shows me that the drone has slowed down. It must have lost power. Go! Go! It should be now much easier to catch it!"]]

   text[11] = [["It seems the drone has found a way to shield itself from the EM pulses, I think I can adjust to that, give me a second."]]
   text[12] = [["There you go! I am a genius after all! Get it!"]]
   text[13] = [["There is something strange, the engines are starting to heat up... if they continue like this the drone will explode in about 20 seconds... You better hurry!"]]
   text[14] = [["NOOOOOOOOO! My drone! You incompetent rice cracker! You failed me!"]]
   -- final msg when returning to gastan
   text[15] = [["The things I do for science! Now let me go back to my lab and analyze the drone. I need to figure out exactly what happened and what went wrong. Once I know more I might need you again. Oh, and here, for your service!" Another payslip and another flight that convinces you that Dr. Geller might be in need of a doctor.]]

   -- text if the mission is failed
   fail_text = [["NOOOOOO! What have you done! My prototype! It's going to take me weeks to rebuild it! You incompetent... rice cracker!!!"]]
   -- osd_msg
   osd_msg[1] = "Go to the %s system and hail the prototype."
   osd_msg[2] = "Disable to prototype."
   osd_msg[3] = "Return the prototype to %s in the %s system."
  -- refuestext 
   refusetitle = "No Science Today"
   refusetext = "I guess you don't care that much about science..."
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
   tk.msg( title[1], text[2] )
   misn.accept()
   misn.osdCreate(misn_title, {osd_msg[1]:format(t_sys[1]),osd_msg[2],osd_msg[3]:format(t_pla[2],t_sys[2])})
   misn.setDesc(misn_desc)
   misn.setTitle(misn_title)
   misn.osdActive(1)
   mmarker = misn.markerAdd(system.get(t_sys[1]), "high")
   hook.jumpin("sys_enter")
end

function sys_enter ()
   -- Check to see if reaching target system
   if system.cur() == system.get(t_sys[1]) then
      -- wait til stars have settled and do stuff
      --pilot.clear()
      --pilot.toggleSpawn(false)
      hook.timer( 2000,"game_of_drones")
   end
   if jumps ~= 0 then 
      if system.cur() == system.get(t_sys[3]) then
         -- do something else
         hook.timer(2000,"chase_of_drones")
      end
   end
   
end

function game_of_drones ()
   tk.msg(title[2], text[3]:format(t_pla[1]:name()))
   -- spawn drones 

   t_drone = pilot.add("Za'lek Scout Drone", "trader",t_pla[1] )[1] -- prototype is a scout drone
   t_drone:addOutfit("Tricon Zephyr II Engine")
   -- add sth so it is not insta disable with one shot?
   --t_drone:addOutfit("Tricon Zephyr II Engine")
   t_drone:setFaction("Civilian")
   t_drone:rename("Za'lek Prototype")
   t_drone:setInvincible(true)
   t_drone:control()
   t_drone:setHilight(true)
   t_drone:setVisplayer(true)
   t_drone:goto(t_drone:pos() + vec2.new( 400, -400), false)
   -- just some moving around, stolen from baron missions ;D
   idlehook = hook.pilot(t_drone, "idle", "targetIdle")
   misn.osdActive(2)
   -- wait for the drone to be hailed
   dist = 200
   sp = t_pla[1] 
   -- + vec2.new(dist*math.cos(math.pi*2*rnd.rnd()),dist*math.sin(2*math.pi*rnd.rnd()))
   --pilot.toggleSpawn(true)
   hook.pilot(t_drone,"hail","got_hailed",t_drone,badguys)
end

function got_hailed()
   if vec2.dist(player.pos(),t_drone:pos()) > 1000 then
      player.msg("Target out of range")
      return
   end
   hook.rm(idlehook)
   tk.msg(title[3], text[4])
   tk.msg(title[3], text[5]:format(t_pla[1]:name()))
   tk.msg(title[3], text[6])
   t_drone:setInvincible(false)
   t_drone:setHostile(true)
   t_drone:setHilight(true)
   t_drone:setVisplayer(true)
   spawnhook = hook.timer(3000, "sp_baddies")
   hook.pilot(t_drone, "death", "failed")
   hook.pilot(t_drone, "board", "targetBoard")
   hook.pilot(t_drone, "jump", "drone_jumped")

end

function sp_baddies()
   hook.rm(spawnhook)
   badguys = {}
   badguys = spawn_baddies(t_pla[1])
   for i=1,#badguys do
     badguys[i]:setHostile(true)
   end
   bghook = {}
   for i=1,#badguys do
      bghook[i] = hook.pilot(badguys[i], "exploded", "dead_drone",i)
   end
   jps = system.cur():jumps()
   t_drone:taskClear()
   t_sys[3] = jps[1]:dest():name()
   t_drone:hyperspace(jps[1]:dest())
end

function failed ()
   tk.msg(title[2],fail_text)
   misn.finish(false)
end

function targetBoard()
   player.unboard()
   tk.msg(title[3], text[7])
   t_drone:setHilight(false)
   t_drone:setVisplayer(false)
   hook.rm(idlehook)
   t_drone:rm()
   cargoID = misn.cargoAdd("Prototype",10)
   misn.osdActive(3)
   misn.markerRm(mmarker)
   mmarker = misn.markerAdd(system.get(t_sys[2]),"high")
   if jumps == 0 then
      leavehook = hook.timer(2000, "drones_flee")
   end
   hook.land("land_home")
end

function drone_jumped ()
   --begin the chase: 
   tk.msg(title[3],text[9]:format(t_sys[3]))
   misn.markerRm(mmarker)
   if (jumps==0) then
      mmarker = misn.markerAdd(system.get(t_sys[3]),"high")
      for i=1,#badguys do
         badguys[i]:control()
         badguys[i]:hyperspace()
         hook.rm(bghook[i])
      end
      if not fled then
         tk.msg(title[3],text[8])
      end
      fled = false
   elseif jumps==2 then
      t_drone:setHealth(0,0)
      tk.msg(title[3],text[14])
      misn.finish(false)
   else
      mmarker = misn.markerAdd(system.get(t_sys[3]),"high")
   end
   jumps = jumps +1
end

-- the drone behaves differently depending on through how many systems it has been chased so far
function chase_of_drones ()
   tk.msg(title[3],text[10])
   t_drone = pilot.add("Za'lek Scout Drone", "dummy",vec2.newP(rnd.rnd(0,system.cur():radius()/5),rnd.rnd(0,359)))[1] -- prototype is a scout drone
   t_drone:addOutfit("Tricon Zephyr II Engine")
   -- add sth so it is not insta disable with one shot?
   t_drone:setFaction("Civilian")
   t_drone:rename("Za'lek Prototype")
   t_drone:control()
   t_drone:setHilight(true)
   t_drone:setVisplayer(true)
   t_drone:goto(t_drone:pos() + vec2.new( 400, -400), false)
   t_stats = t_drone:stats()
   t_drone:setHealth(50,100)
   t_drone:setNodisable()
   dr_a_hook = hook.pilot(t_drone, "attacked", "drone_attacked")
   dr_d_hook = hook.pilot(t_drone, "death", "failed")
   hook.pilot(t_drone, "board", "targetBoard")
   hook.pilot(t_drone, "jump", "drone_jumped")
   if jumps == 1 then
      t_drone:setSpeedLimit(t_stats.speed_max*2/3)
   elseif jumps == 2 then
      t_drone:setSpeedLimit(t_stats.speed_max*1/3)
   elseif jumps >2 then
      player.msg("Something went terribly wrong here! If you see this message please reload and report a bug in this mission. Thank you!")
   end
   -- just some moving around, stolen from baron missions ;D
   idlehook = hook.pilot(t_drone, "idle", "targetIdle")
   
end

function drone_attacked()
   hook.rm(dr_a_hook)
   tk.msg(title[3],text[11])
   t_drone:setHostile(true)
   t_drone:setHilight(true)
   t_drone:setVisplayer(true)
   jps = system.cur():jumps()
   t_drone:taskClear()
   t_sys[3] = jps[1]:dest():name()
   t_drone:hyperspace(jps[1]:dest())
   hook.timer(4000,"drone_disableable")
end

function drone_selfdestruct()
   hook.rm(dr_d_hook)
   t_drone:setHealth(0,0)
   tk.msg(title[3],text[14])
   misn.finish(false)
end

function drone_disableable()
   tk.msg(title[3],text[12])
   t_drone:setNodisable(false)
   if jumps == 2 then
      tk.msg(title[3],text[13])
      hook.timer(18000+rnd.rnd(1,4000),"drone_selfdestruct")
   end
end
-- last hook
function land_home()
   if planet.cur() == planet.get(t_pla[2]) then
      tk.msg(title[4]:format(t_pla[2]),text[15])
      player.pay(reward)
      -- pay player and do shit here
      misn.finish(true)
   end
end

-- tell drones to spread and flee
function drones_flee ()
   tk.msg(title[3],text[8])
   fled = true
   for i=1,#badguys do
      pilot.taskClear(badguys[i])
      badguys[i]:control(true)
      --badguys[i]:changeAI("flee")
      badguys[i]:hyperspace()
   end
   for i=1,#bghook do
      hook.rm(bghook[i])
   end
end

-- check how man drones are left, tell them to leave if <=2 
function dead_drone ()
   -- remove dead drones
   for i=1,#badguys do
      if not badguys[i]:exists() then
--         player.msg("If close worked")
         table.remove(badguys,i)
         break
      end
   end
   if(#badguys<=2) then
      -- if it is the last attacking drone, make it run away from the player
      badguys[1]:control()
      t_drone:control()
      badguys[2]:control()
      jpt = get_nearest_jump(badguys[1])
      jpt2 = jpt
      jpts = system.cur():jumps()
      for i,j_pt in ipairs(jpts) do
         if j_pt ~= jpt2 then
            jpt2 = j_pt
            break
         end
      end
      badguys[1]:hyperspace(jpt2:dest())
      t_drone:hyperspace(jpt:dest())
      badguys[2]:hyperspace()
      tk.msg(title[3],text[8])
      fled = true
      for i=1,#bghook do
         hook.rm(bghook[i])
      end
   end
end

-- general functions

-- keep drone moving
function targetIdle()
   t_drone:goto(t_drone:pos() + vec2.new( 400,  400), false)
   t_drone:goto(t_drone:pos() + vec2.new(-400,  400), false)
   t_drone:goto(t_drone:pos() + vec2.new(-400, -400), false)
   t_drone:goto(t_drone:pos() + vec2.new( 400, -400), false)
   hook.rm(idlehook)
   idlehook = hook.timer(5000, "targetIdle")
end

-- get nearest jumppoint

function get_nearest_jump(pil)
   jpts = system.cur():jumps()
   -- basically the distance that the map can have at 
   dist = 2*system.cur():radius()
   index = 0
   for i,jpt in ipairs(jpts) do
      dist1 = vec2.dist(jpt:pos(),pil:pos())
      if dist1 < dist then
         dist = dist1
         index = i
      end
   end
   return jpts[index]
end
--- create enemy ships
function spawn_baddies(sp)
   -- light drones
   local scom = {}
   -- has eventually to be trimmed
   -- so weird: ai does not work properly apparently? WTF is going on.. switching to dummy and switched from pilot.add to pilot.addRaw
   -- disabling some ships since this way it is really hard to win the mission
   scom[1] = pilot.addRaw("Za'lek Light Drone","mercenary", sp, "Mercenary" )
   scom[2] = pilot.addRaw("Za'lek Light Drone","mercenary", sp, "Mercenary" )
   scom[3] = pilot.addRaw("Za'lek Heavy Drone","mercenary", sp, "Mercenary" )
   scom[4] = pilot.addRaw("Za'lek Heavy Drone","mercenary", sp, "Mercenary" )
--   scom[5] = pilot.addRaw("Za'lek Heavy Drone","mercenary", sp, "Mercenary" )
--   scom[6] = pilot.addRaw("Za'lek Light Drone","mercenary", sp, "Mercenary" )
--   scom[7] = pilot.addRaw("Za'lek Light Drone","mercenary", sp, "Mercenary" )
   for i=1,#scom do
     scom[i]:setHostile(false)
   end

   return scom
end
