--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Collective Espionage 2">
 <unique />
 <priority>2</priority>
 <cond>faction.playerStanding("Empire") &gt; 5 and var.peek("collective_fail") ~= true</cond>
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

   Third mission in the collective mini campaign.

   You must land on an ex-empire planet in collective territory and return.

]]--
local fleet = require "fleet"
local emp = require "common.empire"
local fmt = require "format"
local cinema = require "cinema"

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

   misn.setNPC( _("Dimitri"), "empire/unique/dimitri.webp", _("You notice Lt. Commander Dimitri at one of the booths.") )
end


function accept ()
   -- Intro text
   if not tk.yesno( _("Collective Espionage"), fmt.f(_([[You head over to Lt. Commander Dimitri to see what the results are.
    "Hello there again, {player}. Bad news on your latest run, you got nothing other than the usual robotic chatter. We'll have to send you out again, but this time we'll follow a different approach. Interested in giving it another shot?"]]), {player=player.name()}) ) then
      misn.finish()
   end

   -- Accept the mission
   misn.accept()

   mem.misn_stage = 0
   mem.misn_marker = misn.markerAdd( misn_target, "low" )

   -- Mission details
   misn.setTitle(_("Collective Espionage"))
   misn.setReward( fmt.credits( emp.rewards.ec02 ) )
   misn.setDesc( fmt.f(_("Land on {pnt} in the {sys} system to monitor Collective communications"), {pnt=misn_target, sys=misn_target_sys} ))
   misn.osdCreate(_("Collective Espionage"), {
      fmt.f(_("Fly to {sys} and land on {pnt}"), {sys=misn_target_sys, pnt=misn_target}),
      fmt.f(_("Return to {pnt} with your findings"), {pnt=misn_base}),
   })

   tk.msg( _("Collective Espionage"), fmt.f(_([["On your last run, you were monitoring while out in the open. While you do get better signals, upon noticing your presence, the drones will go into combat mode, and yield only combat transmissions. This mission will consist of hiding and monitoring from a safer spot, hopefully catching them more relaxed.
    "When the Collective struck, they quickly took many systems; one of the bigger losses was {pnt}, an important gas giant rich in methane. They destroyed the gas refineries and slaughtered the humans. There was nothing we could do. The turbulence and dense atmosphere there should be able to hide your ship."]]), {pnt=misn_target}) )
   tk.msg( _("Collective Espionage"), fmt.f(_([["The plan is to have you infiltrate Collective space alone to not arouse too much suspicion. Once inside, you should head to {pnt} in the {sys} system. Stay low and monitor all frequencies in the system. If anything is suspicious, we'll surely catch it then. Don't forget to make sure you have the four jumps of fuel to be able to get there and back in one piece.
    "Good luck, I'll be waiting for you on your return."]]), {pnt=misn_target, sys=misn_target_sys}) )

   hook.land("land")
end

function land()
   -- You land on the planet, but you also immediately take off again.
   if mem.misn_stage == 0 and spob.cur() == misn_target then
      -- Initiate cutscene
      mem.takeoffhook = hook.takeoff("takeoff")
      player.takeoff()

   -- Return bit
   elseif mem.misn_stage == 1 and spob.cur() == misn_base then
      tk.msg( _("Mission Accomplished"), _([[As your ship touches ground, you see Lt. Commander Dimitri come out to greet you.
    "How was the weather?" he asks jokingly. "Glad to see you're still in one piece. We'll get right on analyzing the data acquired. Those robots have to be up to something. Meet me in the bar later. Meanwhile, give yourself a treat; you've earned it. We've made a 700K credit deposit into your bank account. Enjoy it."]]) )

      -- Rewards
      faction.modPlayerSingle("Empire",5)
      player.pay( emp.rewards.ec02 )

      emp.addCollectiveLog( _([[You monitored Collective communications for the Empire again, this time while landed on Eiroik. Lt. Commander Dimitri told you to meet him in the bar on Omega Enclave again later.]]) )

      misn.finish(true)
   end
end

function takeoff()
    -- Build the actual cutscene
    player.pilot():setHide(true)
    cinema.on()

    -- Sinister music landing
    music.play("landing_sinister.ogg")

    -- Some text
    tk.msg( _("Collective Espionage"), fmt.f(_([[You quickly land on {pnt} and hide in its deep, dense methane atmosphere. Your monitoring gear flickers into action, hopefully catching something of some use. With some luck, there won't be too many Collective drones when you take off.]]), {pnt=misn_target}) )
    misn.setDesc( fmt.f(_("Travel back to {pnt} in {sys}"), {pnt=misn_base, sys=misn_base_sys} ))

    local sml_swarm = { "Drone", "Drone", "Drone", "Heavy Drone" }

    swarm1 = fleet.add( 1, sml_swarm, "Collective", vec2.new(-11000, 4000), _("Collective Drone") )
    swarm1[4]:rename(_("Collective Heavy Drone"))
    moveSwarm(swarm1, vec2.new(-8000, -7500))
    swarm2 = fleet.add( 1, sml_swarm, "Collective", vec2.new(1700, 12000), _("Collective Drone") )
    swarm2[4]:rename(_("Collective Heavy Drone"))
    moveSwarm(swarm2, vec2.new(7000, -5000))
    swarm3 = fleet.add( 1, sml_swarm, "Collective", vec2.new(17000, 2500), _("Collective Drone") )
    swarm3[4]:rename(_("Collective Heavy Drone"))
    moveSwarm(swarm3, vec2.new(-9500, 13000))

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
    tk.msg(_("Collective Espionage"), _([[That should be enough. Time to report your findings.]]))
    mem.misn_stage = 1
    misn.markerMove( mem.misn_marker, misn_base )
    player.pilot():setHide(false)
    cinema.off()
    misn.osdActive(2)
    music.choose("ambient")
end
