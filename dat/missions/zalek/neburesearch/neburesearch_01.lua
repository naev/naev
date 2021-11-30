--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Advanced Nebula Research">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <done>Novice Nebula Research</done>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Vilati Vilata</planet>
 </avail>
 <notes>
  <campaign>Nebula Research</campaign>
 </notes>
</mission>
--]]
--[[

   Mission: Advanced Nebula Research

   Description: This time the postdoc asks the player for help. Reference data of further nebula are needed to understand the Sol nebula. The player has to escort a refitted transport ship through Pirate and Dvaered space.

   Difficulty: Medium

]]--
local lmisn = require "lmisn"
local fleet = require "fleet"
local fmt = require "format"
local zlk = require "common.zalek"

local ships, transporter -- Non-persistent state
local fail, spawnTransporter, updateGoalDisplay -- Forward-declared functions
-- luacheck: globals ambushHail jumpin jumpout land startAmbush takeoff timer_transporterSafe transporterAttacked transporterDeath transporterJump transporterLand transporterShutup (Hook functions passed by name)

-- Mission info stuff
local osd_msg   = {}
osd_msg[1] = _("Escort the transport ship to {pnt} in the {sys} system.")
osd_msg[2] = _("Land on {pnt} in the {sys} system.")
osd_msg[3] = _("Fly back to {pnt} in the {sys} system.")

local station = planet.get("PSO Monitor")
local homeworld = planet.get("Bastion Station")
local t_sys = {
    system.get("Ksher"),
    system.get("Sultan"),
    system.get("Faust"),
    system.get("PSO"),
    system.get("Amaroq"),
    system.get("Ksher"),
    system.get("Daravon"),
    system.get("Pultatis"),
}
local t_planet = {
    [1] = planet.get("Qoman"),
    [5] = station,
    [6] = planet.get("Qoman"),
    [7] = planet.get("Vilati Vilata"),
    [8] = homeworld,
}

local credits = 800e3

function create()
    mem.ambush = false
    mem.stage = 0

    -- Spaceport bar stuff
    misn.setNPC(_("A scientist"), "zalek/unique/mensing.webp", _("You see a scientist who is apparently looking for someone."))
end

function accept()
    if not tk.yesno(_("Bar"), fmt.f(_([["Captain {player} if I'm not mistaken? Well met. I heard you recently helped one of our students. My name is Dr. Mensing and I am working for professor Voges as well.
    Your timing is just perfect. You see, we planed an expedition but the captain we hired to escort our transport ship backed out in the last minute. It's quite bothersome being stranded right in Dvaered space. Would you be willing to assist us instead?"]]), {player=player:name()})) then
        misn.finish()
    end

    mem.stage = 1
    mem.exited = false
    mem.firstTakeOff = true
    mem.origin = planet.cur()

    tk.msg(_("Bar"), fmt.f(_([["While the data recorded by Robert is of good quality he seems to have completely forgotten that we need reference data of similarly dense nebulae. We have already installed his sensors on a transport ship. The nearby PSO nebula should be a good candidate but there are the pirate systems in between. Also the target systems are controled by the Dvaered. Hard to say whether the Dvaered or the pirates are more dangerous. So this is why we need an escort.
    We will travel through {sys2}, {sys3}, and {sys4}. Just passing through the systems should be sufficient. Also, I want to visit the {station} station before returning back to {pnt}. You have to make sure no one shoots us down during our expedition."]]), {sys2=t_sys[2], sys3=t_sys[3], sys4=t_sys[4], station=station, pnt=homeworld}))

    -- Set up mission information
    mem.destsys = t_sys[1]
    misn.setTitle(_("Advanced Nebula Research"))
    misn.setReward(fmt.credits(credits))
    misn.setDesc(fmt.f(_("Escort the transport ship to the {station} in the {sys} system. Make sure to stay close to the transport ship and wait until they jumped out of the system safely."), {station=station, sys=t_sys[5]}))
    mem.nextsys = lmisn.getNextSystem(system.cur(), mem.destsys) -- This variable holds the system the player is supposed to jump to NEXT.

    misn.accept()
    mem.misn_marker = misn.markerAdd(mem.nextsys, "low")
    updateGoalDisplay()

    hook.takeoff("takeoff")
    hook.jumpin("jumpin")
    hook.jumpout("jumpout")
    hook.land("land")
end

function updateGoalDisplay()
    local osd_index = {1, 0, 0, 0, 2, 2, 2, 3}
    local omsg = {}
    local osd_active = 1
    for s, i in ipairs(osd_index) do
        if i > 0 then
            omsg[#omsg+1] = fmt.f(osd_msg[i], {pnt=t_planet[s], sys=t_sys[s]})
            if mem.stage > s and (mem.stage > s+1 or mem.destplanet == nil) then
                osd_active = #omsg + 1
            end
        end
    end
    misn.osdCreate(_("Advanced Nebula Research"), omsg)
    misn.osdActive(osd_active)
end

function takeoff()
    if mem.firstTakeOff then
        tk.msg(_("Departure"), fmt.f(_([["Please follow us, {player}. Make sure to jump to the next system after we jumped out. We'll have to land on some planets on our way to refuel."]]), {player=player:name()}))
        mem.firstTakeOff = false
    end
    mem.destplanet = nil
    spawnTransporter()
    updateGoalDisplay()
end

function jumpin()
     if system.cur() ~= mem.nextsys then
        fail(_("MISSION FAILED! You jumped into the wrong system. You failed science miserably!"))
    else
        mem.nextsys = lmisn.getNextSystem(system.cur(), mem.destsys)
        updateGoalDisplay()
        spawnTransporter()
        if not mem.ambush and system.cur():faction() == faction.get("Dvaered") and system.cur():jumpDist(t_sys[5]) < 5 then
            hook.timer(2.0, "startAmbush")
        elseif system.cur()==system.get("Daan") or system.cur()==system.get("Provectus Nova") then
            local ambushers = fleet.add( 1,  {"Pirate Ancestor", "Pirate Vendetta", "Pirate Vendetta", "Pirate Vendetta", "Pirate Hyena", "Pirate Hyena"}, "Pirate", vec2.new(0,0), nil, {ai="baddie_norun"} )
            for i, j in ipairs(ambushers) do
                j:setHostile()
                j:control(true)
                j:attack(transporter)
            end
        end
    end
end

function jumpout()
    if not mem.exited then
        fail(_("MISSION FAILED! You jumped before the transport ship you were escorting."))
    end
    mem.origin = system.cur()
    if mem.nextsys == t_sys[mem.stage] then
        if t_planet[mem.stage] ~= nil then
            mem.destplanet = t_planet[mem.stage]
        else
            mem.destplanet = nil
        end
        mem.stage = mem.stage+1
        mem.destsys = t_sys[mem.stage]
    end
end

function land()
    if not mem.exited then
        tk.msg(_("You abandoned your mission!"), _("You have landed, abandoning your mission to escort the transport ship. You failed science miserably!"))
        misn.finish(false)
    elseif planet.cur() == station and not mem.station_visited then
        tk.msg(_("A short break"), fmt.f(_([[Once you are done with the refuel operations, you meet Dr. Mensing on her way back to the transport ship.
    "I just met up with another 'scientist' working on this station. The purpose of this station is to collect data about the PSO nebula, but their scans are absolute garbage. Apparently the station is being run by an independent university. They couldn't possible keep up with the Za'lek standards in terms of proper scientific methods."
    She is visibly upset about the apparent lack of dedication to science. "Let's head back to {pnt}. Our own measurements are completed by now."]]), {pnt=homeworld}))
        mem.station_visited = true
    elseif planet.cur() == homeworld then
        tk.msg(_("Mission accomplished"), fmt.f(_([[After leaving the ship you meet up with Dr. Mensing who hands you over a chip worth {credits} and thanks you for your help.
    "We'll be able to return to Jorla safely from here on. You did science a great favor today. I'm sure the data we collected will help us to understand the cause for the Sol nebula's volatility."]]), {credits=fmt.credits(credits)}))
        player.pay(credits)
        zlk.addNebuResearchLog(_([[You helped Dr. Mensing to collect sensor data of the PSO nebula.]]))
        misn.finish(true)
    end
    mem.origin = planet.cur()
end

local function continueToDest(pilot)
    if pilot ~= nil and pilot:exists() then
        pilot:control(true)
        pilot:setNoJump(false)
        pilot:setNoLand(false)
        if mem.destplanet ~= nil then
            pilot:land(mem.destplanet, true)
            misn.markerMove(mem.misn_marker, mem.destplanet)
        else
            pilot:hyperspace(mem.nextsys, true)
            misn.markerMove(mem.misn_marker, mem.nextsys)
        end
    end
end

function transporterJump(p, j)
    mem.exited = true
    if p:exists() then
        player.msg(fmt.f(_("{plt} has jumped to {sys}."), {plt=p, sys=j:dest()}))
    end
end

function transporterLand(p, _j)
    mem.exited = true
    if p:exists() then
        player.msg(fmt.f(_("{plt} has landed on {pnt}."), {plt=p, pnt=mem.destplanet}))
    end
end

function transporterAttacked(p, attacker)
    mem.unsafe = true
    p:control(true)
    p:setNoJump(true)
    p:setNoLand(true)
    p:attack(attacker)
    p:control(false)

    if not mem.shuttingup then
        mem.shuttingup = true
        p:comm(player.pilot(), _("We're under attack!"))
        hook.timer(5.0, "transporterShutup") -- Shuts him up for at least 5s.
    end
end

function transporterShutup()
     mem.shuttingup = false
end

function timer_transporterSafe()
    hook.timer(2.0, "timer_transporterSafe")

    if mem.unsafe then
        mem.unsafe = false
        continueToDest(transporter)
    end
end

function spawnTransporter()
    transporter = pilot.add( "Rhino", "Za'lek", mem.origin, _("Nebula Research Shuttle") )
    hook.pilot(transporter, "death", "transporterDeath")
    hook.pilot(transporter, "jump", "transporterJump")
    hook.pilot(transporter, "land", "transporterLand")
    hook.pilot(transporter, "attacked", "transporterAttacked", transporter)
    transporter:control()
    transporter:setInvincPlayer()
    transporter:setHilight(true)
    transporter:setVisplayer()
    transporter:setVisible() -- Hack to make ambushes more reliable.
    transporter:setFriendly()
    transporter:rename( _("Research Shuttle") )
    continueToDest(transporter)
    hook.timer( 2.0, "timer_transporterSafe" )
end

function startAmbush()
    ships = fleet.add( 1,  {"Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Ancestor", "Dvaered Ancestor"}, "Dvaered", vec2.new(-1000,0), nil, {ai="dvaered_norun"} )
    for i, j in ipairs(ships) do
        j:control(true)
        j:moveto(vec2.new(-8000,0))
    end
    mem.ambush = true
    hook.timer(15.0, "ambushHail")
end

function ambushHail()
    for i, j in ipairs(ships) do
        j:setHostile()
        j:setHilight()
        j:control(true)
        j:attack(transporter)
        j:setVisible()
        j:setVisplayer()
    end
    tk.msg(_("Trouble inbound"), _([[Suddenly your comm system turns on, receiving a conversation between the ship you are escorting and a Dvaered patrol ship.
    "I assure you, we mean no harm. We are just a convoy of scientists passing through Dvaered space." says Dr. Mensing.
    The response sounds harsh. "Do you think we're naïve? You're obviously a spy scouting our systems' defences."
    "Please calm down. I'm sure there is a diplomatic solution for our misunderstanding."
    The Dvaered officer replies "We can see that your ship is stuffed with sensors. Your intentions are obvious. Prepare for your ship being boarded."]]))
    tk.msg(_("Trouble inbound"), _([[Dr. Mensing  pauses, apparently choosing her words with care.
    "Fine, do whatever you want. Our reasoning is obviously beyond the imagination of your degenerate intellect." With this answer the comm shuts off. Your sensors show that a Dvaered patrol changed their course and is heading straight towards the transporter.]]))
    tk.msg(_("Trouble inbound"), fmt.f(_([["The situation would have escalated anyway." argues Dr. Mensing, this time directly speaking towards you.
    "I must admit, it is suspicious for a refitted transport ship with such advanced sensor suits to show up in Dvaered space. I hadn't considered this point.
    I'm counting on you, {player}. Please help us."]]), {player=player:name()}))
end

-- Handle the destruction of the transporter. Abort the mission.
function transporterDeath()
    tk.msg(_("The transporter was destroyed!"), _("The transporter was destroyed and all scientists died! Even worse, you failed science!"))
    misn.finish(false)
end

-- Fail the mission, showing message to the player.
function fail( message )
   if message ~= nil then
      -- Pre-colourized, do nothing.
      if message:find("#") then
         player.msg( message )
      -- Colourize in red.
      else
         player.msg( "#r" .. message .. "#0" )
      end
   end
   misn.finish( false )
end
