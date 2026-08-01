--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Collective Espionage 2">
 <unique />
 <priority>2</priority>
 <cond>faction.reputationGlobal("Empire") &gt; 5 and var.peek("collective_fail") ~= true</cond>
 <done>Collective Espionage 1</done>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Omega Enclave</spob>
 <notes>
  <campaign>Collective</campaign>
 </notes>
</mission>
--]]
--[[

   Collective Espionage II

   Author: bobbens
      minor edits by Infiltrator
      converted to VN framework by MageKing17

   Third mission in the collective mini campaign.

   You must land on an ex-empire planet in collective territory and return.

]]--
local fleet = require "fleet"
local emp = require "common.empire"
local fmt = require "format"
local cinema = require "cinema"

local vn = require "vn"
local lmusic = require "lmusic"

-- Mission consstants
local misn_base, misn_base_sys = spob.getS("Omega Enclave")
local misn_target, misn_target_sys = spob.getS("Eiroik")

local swarm1, swarm2, swarm3 -- Non-persistent state
local moveSwarm -- Forward-declared functions

function create ()
   local missys = {misn_target}
   if not misn.claim(missys) then
      misn.finish(false)
   end

   misn.setNPC( _("Dimitri"), emp.dimitri.portrait, emp.dimitri.description )
end


function accept ()
   local accepted = false

   vn.reset()
   vn.scene()
   local dimitri = vn.newCharacter(emp.vn_dimitri())
   vn.transition(emp.dimitri.transition)
   dimitri(fmt.f(_([[You head over to Lt. Commander Dimitri to see what the results are.
"Hello there again, {player}. Bad news on your latest run, you got nothing other than the usual robotic chatter. We'll have to send you out again, but this time we'll follow a different approach. Interested in giving it another shot?"]]), {player=player.name()}))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }
   vn.label("decline")
   vn.done(emp.dimitri.transition)

   vn.label("accept")
   vn.func(function () accepted = true end)
   dimitri(_([["On your last run, you were monitoring while out in the open. While you do get better signals, upon noticing your presence, the drones will go into combat mode, and yield only combat transmissions. This mission will consist of hiding and monitoring from a safer spot, hopefully catching them more relaxed."]]))
   dimitri(fmt.f(_([["When the Collective struck, they quickly took many systems; one of the bigger losses was {pnt}, an important gas giant rich in methane. They destroyed the gas refineries and slaughtered the humans. There was nothing we could do. The turbulence and dense atmosphere there should be able to hide your ship."]]), {pnt=misn_target}))
   dimitri(fmt.f(_([["The plan is to have you infiltrate Collective space alone to not arouse too much suspicion. Once inside, you should head to {pnt} in the {sys} system. Stay low and monitor all frequencies in the system. If anything is suspicious, we'll surely catch it then."]]), {pnt=misn_target, sys=misn_target_sys}))
   dimitri(_([["Don't forget to make sure you have the four jumps of fuel to be able to get there and back in one piece. Good luck, I'll be waiting for you on your return."]]))
   vn.done(emp.dimitri.transition)
   vn.run()

   if not accepted then return end

   -- Accept the mission
   misn.accept()

   mem.misn_stage = 0
   mem.misn_marker = misn.markerAdd( misn_target, "low" )

   -- Mission details
   misn.setTitle(_("Collective Espionage"))
   misn.setReward( emp.rewards.ec02 )
   misn.setDesc( fmt.f(_("Land on {pnt} in the {sys} system to monitor Collective communications"), {pnt=misn_target, sys=misn_target_sys} ))
   misn.osdCreate(_("Collective Espionage"), {
      fmt.f(_("Fly to {sys} and land on {pnt}"), {sys=misn_target_sys, pnt=misn_target}),
      fmt.f(_("Return to {pnt} with your findings"), {pnt=misn_base}),
   })

   hook.land("land")
end

function land()
   -- You land on the planet, but you also immediately take off again.
   if mem.misn_stage == 0 and spob.cur() == misn_target then
      -- Initiate cutscene
      mem.takeoffhook = hook.takeoff("takeoff")
      -- Stop the default music change from taking off.
      music.stop(true)
      player.takeoff()

   -- Return bit
   elseif mem.misn_stage == 1 and spob.cur() == misn_base then
      vn.reset()
      vn.scene()
      local dimitri = vn.newCharacter(emp.vn_dimitri())
      vn.transition(emp.dimitri.transition)
      vn.na(_([[As your ship touches ground, you see Lt. Commander Dimitri come out to greet you.]]))
      dimitri(_([["How was the weather?" he asks jokingly. "Glad to see you're still in one piece. We'll get right on analysing the data acquired. Those robots have to be up to something. Meet me in the bar later."]]))
      dimitri(_([["Meanwhile, give yourself a treat; you've earned it. We've made a deposit into your bank account. Enjoy it."]]))
      vn.func(function ()
         faction.hit("Empire", 35)
         player.pay(emp.rewards.ec02)
      end)
      vn.sfxVictory()
      vn.na(fmt.reward(emp.rewards.ec02))
      vn.done(emp.dimitri.transition)
      vn.run()

      emp.addCollectiveLog(fmt.f(_([[You monitored Collective communications for the Empire again, this time while landed on Eiroik. Lt. Commander Dimitri told you to meet him in the bar on {pnt} again later.]]), {pnt=misn_base}))

      misn.finish(true)
   end
end

function takeoff()
   -- Build the actual cutscene
   player.pilot():setHide(true)
   cinema.on()

   -- Sinister music landing
   -- Can't use vn.music() because we want it to keep playing until the cutscene is over.
   lmusic.play("snd/music/landing_sinister")

   vn.reset()
   vn.scene()
   vn.transition()
   vn.na(fmt.f(_([[You quickly land on {pnt} and hide in its deep, dense methane atmosphere. Your monitoring gear flickers into action, hopefully catching something of some use. With some luck, there won't be too many Collective drones when you take off.]]), {pnt=misn_target}))
   vn.done()
   vn.run()

   misn.setDesc( fmt.f(_("Travel back to {pnt} in {sys}"), {pnt=misn_base, sys=misn_base_sys} ))

   local sml_swarm = { "Drone", "Drone", "Drone", "Heavy Drone" }
   local mts=misn_target_sys

   swarm1 = fleet.add( 1, sml_swarm, "Collective",mts:waypoints("ec02_src1") , _("Collective Drone") )
   swarm1[4]:rename(_("Collective Heavy Drone"))
   moveSwarm(swarm1, mts:waypoints("ec02_dst1"))
   swarm2 = fleet.add( 1, sml_swarm, "Collective",mts:waypoints("ec02_src2") , _("Collective Drone") )
   swarm2[4]:rename(_("Collective Heavy Drone"))
   moveSwarm(swarm2, mts:waypoints("ec02_dst2"))
   swarm3 = fleet.add( 1, sml_swarm, "Collective",mts:waypoints("ec02_src3") , _("Collective Drone") )
   swarm3[4]:rename(_("Collective Heavy Drone"))
   moveSwarm(swarm3,mts:waypoints("ec02_dst3") )

   local delay = 1.0
   hook.timer(delay, "cameraZoom", {targ = swarm1[1], speed = 5000})
   delay = delay + 8.0
   hook.timer(delay, "cameraZoom", {targ = swarm2[1], speed = 5000})
   delay = delay + 8.0
   hook.timer(delay, "cameraZoom", {targ = swarm3[1], speed = 5000})
   delay = delay + 8.0
   hook.timer(delay, "cameraZoom", {targ = nil, speed = 5000})
   delay = delay + 4.0
   hook.timer(delay, "endCutscene")

   hook.rm(mem.takeoffhook)
end

function cameraZoom(args)
   local targ = args.targ
   local speed = args.speed
   camera.set(targ, false, speed)
end

function moveSwarm(flt, pos)
   local dpos = pos - flt[1]:pos()
   for _, j in ipairs(flt) do
      if j:exists() then
         j:control()
         j:setVisplayer(true)
         j:moveto(j:pos() + dpos, false)
      end
   end
end

local function removeSwarm(flt)
   for _, j in ipairs(flt) do
      if j:exists() then
         j:rm()
      end
   end
end

function endCutscene()
   removeSwarm(swarm1)
   removeSwarm(swarm2)
   removeSwarm(swarm3)

   vn.reset()
   vn.scene()
   vn.transition()
   vn.na(_([[That should be enough. Time to report your findings.]]))
   vn.done()
   vn.run()

   mem.misn_stage = 1
   misn.markerMove( mem.misn_marker, misn_base )
   player.pilot():setHide(false)
   cinema.off()
   misn.osdActive(2)
   music.choose("ambient")
   lmusic.clear()
end
