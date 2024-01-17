--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The one with the Runaway">
 <unique />
 <priority>4</priority>
 <done>The one with the Visit</done>
 <chance>100</chance>
 <location>Bar</location>
 <faction>Za'lek</faction>
 <cond>spob.cur() == require("common.sciencegonewrong").getCenterOperations()</cond>
</mission>
--]]
--[[
   Mission: The one with the Runaway
   Description: A prototype drone runs away and needs to be caught by the player
   How to make this more interesting than just go to a system and shoot down the drone w ion or medusa? The ship has to have enough cargo for the drone to be transported back.
   So the player has to hail the drone bc the scientist wants to patch the software which fails and the drone causes other drones to attack.
   I was thinking of player having to use electronic warfare. But is there a way to check that?
   Difficulty: relatively easy?

   Author: fart but based on Mission Ideas in wiki: wiki.naev.org/wiki/Mission_Ideas
--]]
local sciwrong = require "common.sciencegonewrong"
local fmt = require "format"
local vn = require "vn"
local lmisn = require "lmisn"

local badguys, bghook, t_drone -- Non-persistent state

-- system with the drone and the return to start
mem.t_sys = {}
mem.t_pla = {}
-- Mission details
local reward = 2e6
local reward_outfit = outfit.get("Toy Drone")
-- amount of mem.jumps the drone did to escape. Each jump reduces it's speed
mem.fled = false
mem.jumps = 0
mem.t_sys[1] = system.get("Xavier")
mem.t_pla[1] = mem.t_sys[1]:spobs()[1]
--mem.t_pla[2], mem.t_sys[2] = spob.getS("Gastan")

function create ()
   -- Have to be at center of operations.
   mem.t_pla[2], mem.t_sys[2] = sciwrong.getCenterOperations()
   if spob.cur() ~= mem.t_pla[2] then
      misn.finish(false)
   end

   -- Spaceport bar stuff
   misn.setNPC( _("Dr. Geller"),  sciwrong.geller.portrait, _("You see Dr. Geller going from one person to the next, seemingly asking for something.") )
end

function accept()
   local accepted = false
   vn.clear()
   vn.scene()
   local geller = vn.newCharacter( sciwrong.vn_geller() )
   vn.transition()

   -- Mission details:
   geller(_([["Hey there again! I need your help. I was finishing up my prototype, you see. It's ingenious. But, there was a minor hiccup. It's nothing major, it is just, well, that I lost it. But I would not be Dr. Geller if I had not put a tracking mechanism into it! So I want you to catch it and bring it back, OK? You can do that, right?"]]))
   vn.menu( {
      { _("Help them out"), "accept" },
      { _("Decline to help"), "decline" },
   } )

   vn.label("decline")
   geller(_("Don't you care about science?..."))
   vn.done()

   vn.label("accept")
   geller(_([["Excellent! I will join you this time. Let's go."]]) )
   vn.func( function () accepted = true end )
   vn.run()

   if not accepted then
      return
   end

   misn.accept()
   misn.osdCreate(_("The one with the Runaway"), {
      fmt.f(_("Go to the {sys} system and hail the prototype"), {sys=mem.t_sys[1]}),
      _("Disable the prototype"),
      fmt.f(_("Return the prototype to {pnt} in the {sys} system"), {pnt=mem.t_pla[2], sys=mem.t_sys[2]})})
   misn.setDesc(_("You've been hired by Dr. Geller to retrieve his prototype that ran away."))
   misn.setTitle(_("The one with the Runaway"))
   misn.setReward(_("A peek at the new prototype and some compensation for your efforts"))
   mem.mmarker = misn.markerAdd(mem.t_sys[1], "high")
   hook.jumpin("sys_enter")
end

-- general functions

-- get nearest jumppoint
local function get_nearest_jump(pil)
   local jpts = system.cur():jumps(true)
   -- basically the distance that the map can have at
   local dist = 2*system.cur():radius()
   local index = 0
   for i,jpt in ipairs(jpts) do
      local dist1 = vec2.dist(jpt:pos(),pil:pos())
      if dist1 < dist then
         dist = dist1
         index = i
      end
   end
   return jpts[index]
end

local function fdrone ()
   return faction.dynAdd( "Za'lek", "bad_drone", _("Za'lek"), {clear_allies=true, clear_enemies=true} )
end

--- create enemy ships
local function spawn_baddies(sp)
   local fct = fdrone()

   -- light drones
   local scom = {}
   -- has eventually to be trimmed
   -- disabling some ships since this way it is really hard to win the mission
   scom[1] = pilot.add("Za'lek Light Drone", fct, sp )
   scom[2] = pilot.add("Za'lek Light Drone", fct, sp )
   scom[3] = pilot.add("Za'lek Heavy Drone", fct, sp )
   scom[4] = pilot.add("Za'lek Heavy Drone", fct, sp )
--   scom[5] = pilot.add("Za'lek Heavy Drone", fct, sp )
--   scom[6] = pilot.add("Za'lek Light Drone", fct, sp )
--   scom[7] = pilot.add("Za'lek Light Drone", fct, sp )
   for i=1,#scom do
     scom[i]:setHostile(false)
   end

   return scom
end

function sys_enter ()
   -- Check to see if reaching target system
   if system.cur() == mem.t_sys[1] and not mem.captured then
      -- wait til stars have settled and do stuff
      --pilot.clear()
      --pilot.toggleSpawn(false)
      hook.timer( 2.0, "game_of_drones" )
   end
   if mem.jumps ~= 0 then
      if system.cur() == mem.t_sys[3] and not mem.captured then
         -- do something else
         hook.timer( 2.0, "chase_of_drones" )
      end
   end
end

function game_of_drones ()
   vn.clear()
   vn.scene()
   local geller = vn.newCharacter( sciwrong.vn_geller() )
   vn.transition()
   geller(fmt.f(_([["There! The tracker shows it must be here! It is right next to {pnt}! If you hail it I might be able to patch the software. That should give me control again. But you have to be close so the data transfer is as stable as possible."]]), {pnt=mem.t_pla[1]}))
   vn.run()
   -- spawn drones

   t_drone = pilot.add( "Za'lek Scout Drone", fdrone(), mem.t_pla[1], _("Prototype Drone"), {ai="trader"} ) -- prototype is a scout drone
   -- add something so it is not insta-disabled with one shot?
   t_drone:setFaction("Independent")
   t_drone:setInvincible(true)
   t_drone:control()
   t_drone:setHilight(true)
   t_drone:setVisplayer(true)
   t_drone:moveto(t_drone:pos() + vec2.new( 400, -400), false)
   -- just some moving around, stolen from baron missions ;D
   mem.idlehook = hook.pilot(t_drone, "idle", "targetIdle")
   misn.osdActive(1)
   -- wait for the drone to be hailed
   mem.hailhook = hook.pilot(t_drone,"hail","got_hailed",t_drone,badguys)
end

function got_hailed()
   if vec2.dist(player.pos(),t_drone:pos()) > 1000 then
      player.msg(_("Target out of range"))
      player.commClose()
      return
   end
   hook.rm(mem.hailhook)
   hook.rm(mem.idlehook)
   vn.clear()
   vn.scene()
   local geller = vn.newCharacter( sciwrong.vn_geller() )
   vn.transition()
   geller(_([["Huh, I don't understand. This should not be happening. Hold on. I can't get access."]]))
   geller(fmt.f(_([["Um, there seems to be a glitch. Well, sort of. Um, if I deciphered this correctly, the drone just hijacked the unused drones on {pnt} and ordered them to attack us. I never should have tampered with that weird chip those pirates sold me!"]]), {pnt=mem.t_pla[1]}))
   geller(_([["If you can disable the prototype, do it, but I'd prefer not to die at any rate!"]]))
   vn.run()

   misn.osdActive(2)
   t_drone:setInvincible(false)
   t_drone:setHostile(true)
   t_drone:setHilight(true)
   t_drone:setVisplayer(true)
   mem.spawnhook = hook.timer(3.0, "sp_baddies")
   hook.pilot(t_drone, "death", "failed")
   hook.pilot(t_drone, "board", "targetBoard")
   hook.pilot(t_drone, "jump", "drone_jumped")
   player.commClose()
end

function sp_baddies()
   hook.rm(mem.spawnhook)
   badguys = {}
   badguys = spawn_baddies(mem.t_pla[1])
   for i=1,#badguys do
     badguys[i]:setHostile(true)
   end
   bghook = {}
   for i=1,#badguys do
      bghook[i] = hook.pilot(badguys[i], "exploded", "dead_drone",i)
   end
   local jps = system.cur():jumps(true)
   t_drone:taskClear()
   mem.t_sys[3] = jps[1]:dest()
   t_drone:hyperspace(jps[1]:dest())
end

function failed ()
   vn.clear()
   vn.scene()
   local geller = vn.newCharacter( sciwrong.vn_geller() )
   vn.transition()
   geller(_([["NOOOOOO! What have you done!? My prototype! It's going to take me weeks to rebuild it! You incompetent nincompoop!"]]))
   vn.run()
   lmisn.fail(_("you destroyed the drone!"))
end

function targetBoard()
   player.msg(_([[Geller: "Excellent work! Now load it up and let's get out of here!"]]), true)
   mem.captured = true
   local c = commodity.new(N_("Prototype"), N_("A disabled prototype drone."))
   mem.cargoID = misn.cargoAdd(c, 10)
   misn.osdActive(3)
   misn.markerMove( mem.mmarker, mem.t_pla[2], "high" )
   if mem.jumps == 0 then
      hook.timer(2.0, "drones_flee")
   end
   hook.land("land_home")

   player.unboard()
   t_drone:rm()
end

function drone_jumped ()
   --begin the chase:
   player.msg(fmt.f(_([[Geller: "The drone has disappeared from my radar! It must have jumped to the {sys} system. Let's find it!"]]), {sys=mem.t_sys[3]}), true)
   misn.markerRm(mem.mmarker)
   if (mem.jumps==0) then
      mem.mmarker = misn.markerAdd(mem.t_sys[3], "high")
      for i=1,#badguys do
         badguys[i]:control()
         badguys[i]:hyperspace()
         hook.rm(bghook[i])
      end
      if not mem.fled then
         player.msg(_([[Geller: "Interesting, the other drones are running away..."]]), true)
      end
      mem.fled = false
   elseif mem.jumps == 2 then
      t_drone:setHealth(0,0)
      player.msg(_([[Geller: "NOOOOOOOOO! My drone! You imbecile! You failed me!"]]), true)
      lmisn.fail(_("you failed to recover the drone!"))
   else
      mem.mmarker = misn.markerAdd(mem.t_sys[3], "high")
   end
   mem.jumps = mem.jumps + 1
end

-- the drone behaves differently depending on through how many systems it has been chased so far
function chase_of_drones ()
   vn.clear()
   vn.scene()
   local geller = vn.newCharacter( sciwrong.vn_geller() )
   vn.transition()
   geller(_([["The scanner shows me that the drone has slowed down. It must have lost power. Go! Go! Now it should be much easier to catch it!"]]))
   vn.run()

   t_drone = pilot.add(
      "Za'lek Scout Drone",
      fdrone(),
      vec2.newP(rnd.rnd(0,system.cur():radius()/5), rnd.angle()),
      _("Prototype Drone"),
      {ai="dummy"}
   )
   -- add something so it is not insta-disabled with one shot?
   t_drone:setFaction("Independent")
   t_drone:control()
   t_drone:setHilight(true)
   t_drone:setVisplayer(true)
   t_drone:moveto(t_drone:pos() + vec2.new( 400, -400), false)
   local t_stats = t_drone:stats()
   t_drone:setHealth(50,100)
   t_drone:setNoDisable()
   mem.dr_a_hook = hook.pilot(t_drone, "attacked", "drone_attacked")
   mem.dr_d_hook = hook.pilot(t_drone, "death", "failed")
   hook.pilot(t_drone, "board", "targetBoard")
   hook.pilot(t_drone, "jump", "drone_jumped")
   if mem.jumps == 1 then
      t_drone:setSpeedLimit(t_stats.speed_max*2/3)
   elseif mem.jumps == 2 then
      t_drone:setSpeedLimit(t_stats.speed_max*1/3)
   elseif mem.jumps > 2 then
      player.msg(_("Something went terribly wrong here! If you see this message please reload and report a bug in this mission. Thank you!"))
   end
   -- just some moving around, stolen from baron missions ;D
   mem.idlehook = hook.pilot(t_drone, "idle", "targetIdle")

end

function drone_attacked()
   hook.rm(mem.dr_a_hook)
   vn.clear()
   vn.scene()
   local geller = vn.newCharacter( sciwrong.vn_geller() )
   vn.transition()
   geller(_([["It seems the drone has found a way to shield itself from the EM pulses. I think I can adjust to that, give me a second."]]))
   vn.run()
   t_drone:setHostile(true)
   t_drone:setHilight(true)
   t_drone:setVisplayer(true)
   local jps = system.cur():jumps(true)
   t_drone:taskClear()
   mem.t_sys[3] = jps[1]:dest()
   t_drone:hyperspace(jps[1]:dest())
   hook.timer(4.0, "drone_disableable")
end

function drone_selfdestruct()
   hook.rm(mem.dr_d_hook)
   t_drone:setHealth(0,0)
   player.msg(_([[Geller: "NOOOOOOOOO! My drone! You imbecile! You failed me!"]]), true)
   lmisn.fail(_("the drone self-destructed!"))
end

function drone_disableable()
   tk.msg(_([[On your ship]]),_([["There you go! Get it!"]]))
   t_drone:setNoDisable(false)
   if mem.jumps == 2 then
      vn.clear()
      vn.scene()
      local geller = vn.newCharacter( sciwrong.vn_geller() )
      vn.transition()
      geller(_([["This is strange. The engines are starting to heat up... oh, shit, if they continue like this the drone will explode in about 20 seconds! You'd better hurry!"]]))
      vn.run()
      hook.timer(18.0+rnd.uniform(0.001, 4.0), "drone_selfdestruct")
   end
end
-- last hook
function land_home()
   if spob.cur() == mem.t_pla[2] then
      vn.clear()
      vn.scene()
      local geller = vn.newCharacter( sciwrong.vn_geller() )
      vn.transition()
      geller(_([["The things I do for science! Now let me go back to my lab and analyse the drone. I need to figure out exactly what happened and what went wrong. Once I know more I might need you again. Oh, and here, for your service!" A small bag containing a credit chip and a tiny toy drone is tossed your way.]]))
      vn.sfxVictory()
      vn.func( function ()
         player.pay(reward)
         player.outfitAdd(reward_outfit)
      end )
      vn.na(fmt.reward(reward).."\n"..fmt.reward(reward_outfit))
      vn.run()

      sciwrong.addLog( _([[You helped Dr. Geller retrieve his lost prototype drone.]]) )
      misn.finish(true)
   end
end

-- tell drones to spread and flee
function drones_flee ()
   player.msg(_([[Geller: "Interesting, the other drones are running away..."]]),true)
   mem.fled = true
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

-- check how many drones are left, tell them to leave if <=2
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
      local jpt = get_nearest_jump(badguys[1])
      local jpt2 = jpt
      local jpts = system.cur():jumps(true)
      for i,j_pt in ipairs(jpts) do
         if j_pt ~= jpt2 then
            jpt2 = j_pt
            break
         end
      end
      badguys[1]:hyperspace(jpt2:dest())
      t_drone:hyperspace(jpt:dest())
      badguys[2]:hyperspace()
      player.msg(_([[Geller: "Interesting, the other drones are running away..."]]),true)
      mem.fled = true
      for i=1,#bghook do
         hook.rm(bghook[i])
      end
   end
end

-- keep drone moving
function targetIdle()
   if t_drone:exists() then
      t_drone:moveto(t_drone:pos() + vec2.new( 400,  400), false)
      t_drone:moveto(t_drone:pos() + vec2.new(-400,  400), false)
      t_drone:moveto(t_drone:pos() + vec2.new(-400, -400), false)
      t_drone:moveto(t_drone:pos() + vec2.new( 400, -400), false)
      mem.idlehook = hook.timer(5.0, "targetIdle")
   end
end
