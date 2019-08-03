--[[

   Collective Espionage II

   Author: bobbens
      minor edits by Infiltrator

   Third mission in the collective mini campaign.

   You must land on an ex-empire planet in collective territory and return.

]]--

bar_desc = _("You notice Lt. Commander Dimitri at one of the booths.")
misn_title = _("Collective Espionage")
misn_reward = _("700,000 credits")
misn_desc = {}
misn_desc[1] = _("Land on %s in the %s system to monitor Collective communications.")
misn_desc[2] = _("Travel back to %s in %s.")
title = {}
title[1] = _("Collective Espionage")
title[2] = _("Mission Accomplished")
text = {}
text[1] = _([[You head over to Lt. Commander Dimitri to see what the results are.
    "Hello there again, %s. Bad news on your latest run, you got nothing other than the usual robotic chatter. We'll have to send you out again, but this time we'll follow a different approach. Interested in giving it another shot?"]])
text[2] = _([["On your last run, you were monitoring while out in the open. While you do get better signals, upon noticing your presence, the drones will go into combat mode, and yield only combat transmissions. This mission will consist of hiding and monitoring from a safer spot, hopefully catching them more relaxed."
    "When the Collective struck, they quickly took many systems; one of the bigger hits was %s, an important gas giant rich in methane. They destroyed the gas refineries and slaughtered the humans. There was nothing we could do. The turbulence and dense atmosphere there should be able to hide your ship."]])
text[3] = _([["The plan is to have you infiltrate Collective space alone to not arouse too much suspicion. Once inside, you should head to %s in the %s system. Stay low and monitor all frequencies in the system. If anything is suspicious, we'll surely catch it then. Don't forget to make sure you have the four jumps of fuel to be able to get there and back in one piece."
    "Good luck, I'll be waiting for you on your return."]])
text[4] = _([[You quickly land on %s and hide in its deep dense methane atmosphere. Your monitoring gear flickers into action, hopefully catching something of some use. With some luck there won't be too many Collective drones when you take off.]])
text[5] = _([[That should be enough. Time to report your findings.]])
text[6] = _([[As your ship touches ground, you see Lt. Commander Dimitri come out to greet you.
    "How was the weather?" he asks jokingly. "Glad to see you're still in one piece. We'll get right on analysing the data acquired. Those robots have to be up to something. Meet me in the bar later. Meanwhile give yourself a treat; you've earned it. We've made a 700k credit deposit in your bank account. Enjoy it."]])

osd_msg = {}
osd_msg[1] = _("Fly to %s and land on %s")
osd_msg[2] = _("Return to %s with your findings")
osd_msg["__save"] = true 


function create ()
   misn_base, misn_base_sys = planet.get("Omega Station")
   misn_target, misn_target_sys = planet.get("Eiroik")

    local missys = {misn_target}
    if not misn.claim(missys) then
        abort()
    end

   misn.setNPC( _("Dimitri"), "empire/unique/dimitri" )
   misn.setDesc( bar_desc )
end


function accept ()
   -- Intro text
   if not tk.yesno( title[1], string.format(text[1], player.name()) ) then
      misn.finish()
   end

   -- Accept the mission
   misn.accept()

   misn_stage = 0
   misn_marker = misn.markerAdd( misn_target_sys, "low" )

   -- Mission details
   misn.setTitle(misn_title)
   misn.setReward( misn_reward )
   misn.setDesc( string.format(misn_desc[1], misn_target:name(), misn_target_sys:name() ))
   osd_msg[1] = osd_msg[1]:format(misn_target_sys:name(), misn_target:name())
   osd_msg[2] = osd_msg[2]:format(misn_base:name())
   misn.osdCreate(misn_title, osd_msg)

   tk.msg( title[1], string.format(text[2], misn_target:name()) )
   tk.msg( title[1], string.format(text[3], misn_target:name(), misn_target_sys:name()) )

   hook.land("land")
end

function land()
   -- You land on the planet, but you also immediately take off again.
   if misn_stage == 0 and planet.cur() == misn_target then
      -- Initiate cutscene
      takeoffhook = hook.takeoff("takeoff")
      player.takeoff()
   -- Return bit
   elseif misn_stage == 1 and planet.cur() == misn_base then
      tk.msg( title[2], text[6] )

      -- Rewards
      faction.modPlayerSingle("Empire",5)
      player.pay( 700000 )

      misn.finish(true)
   end
end

function takeoff()
    -- Sinister music landing
    music.load("landing_sinister")
    music.play()

    -- Some text
    tk.msg( title[1], string.format(text[4], misn_target:name()) )
    misn.setDesc( string.format(misn_desc[2], misn_base:name(), misn_base_sys:name() ))

    -- Build the actual cutscene
    player.pilot():setInvisible(true)
    player.cinematics(true)
    swarm1 = pilot.add("Collective Sml Swarm", nil, vec2.new(-11000, 4000))
    moveSwarm(swarm1, vec2.new(-8000, -7500))
    swarm2 = pilot.add("Collective Sml Swarm", nil, vec2.new(1700, 12000))
    moveSwarm(swarm2, vec2.new(7000, -5000))
    swarm3 = pilot.add("Collective Sml Swarm", nil, vec2.new(17000, 2500))
    moveSwarm(swarm3, vec2.new(-9500, 13000))

    local delay = 1000
    hook.timer(delay, "cameraZoom", {targ = swarm1[1], speed = 5000})
    delay = delay + 8000
    hook.timer(delay, "cameraZoom", {targ = swarm2[1], speed = 5000})
    delay = delay + 8000
    hook.timer(delay, "cameraZoom", {targ = swarm3[1], speed = 5000})
    delay = delay + 8000
    hook.timer(delay, "cameraZoom", {targ = nil, speed = 5000})
    delay = delay + 4000
    hook.timer(delay, "endCutscene")

    hook.rm(takeoffhook)
end

function cameraZoom(args)
    local targ = args.targ
    local speed = args.speed
    camera.set(targ, true, speed)
end

function moveSwarm(fleet, pos)
    local dpos = pos - fleet[1]:pos()
    for _, j in ipairs(fleet) do
        if j:exists() then
            j:control()
            j:setVisplayer(true)
            j:goto(j:pos() + dpos, false)
        end
    end
end

function removeSwarm(fleet)
    for _, j in ipairs(fleet) do
        if j:exists() then
            j:rm()
        end
    end
end

function endCutscene()
    removeSwarm(swarm1)
    removeSwarm(swarm2)
    removeSwarm(swarm3)
    tk.msg(title[1], text[5])
    misn_stage = 1
    misn.markerMove( misn_marker, misn_base_sys )
    player.pilot():setInvisible(false)
    player.cinematics(false)
    misn.osdActive(2)
    music.delay("ambient", 0)
end
