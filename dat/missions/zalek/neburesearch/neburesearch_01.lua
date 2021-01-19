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

require "scripts/nextjump"
require "scripts/numstring"
require "missions/zalek/common"


bar_desc = _("You see a scientist who is apparently looking for someone.")
mtitle = _("Advanced Nebula Research")
misn_reward = _("%s")
mdesc = _("Escort the transport ship to the %s in the %s system. Make sure to stay close to the transport ship and wait until they jumped out of the system safely.")
title = {}
title[1] = _("Bar")
title[2] = _("Departure")
title[3] = _("Trouble inbound")
title[4] = _("Scans complete")
title[5] = _("Mission accomplished")
text = {}
text[1] = _([["Captain %s if I'm not mistaken? Well met. I heard you recently helped one of our students. My name is Dr. Mensing and I am working for professor Voges as well.
    Your timing is just perfect. You see, we planed an expedition but the captain we hired to escort our transport ship backed out in the last minute. It's quite bothersome being stranded right in Dvaered space. Would you be willing to assist us instead?"]])
text[2] = _([["While the data recorded by Robert is of good quality he seems to have completely forgotten that we need reference data of similarly dense nebulae. We have already installed his sensors on a transport ship. The nearby PSO nebula should be a good candidate but there are the pirate systems in between. Also the target systems are controled by the Dvaered. Hard to say whether the Dvaered or the pirates are more dangerous. So this is why we need an escort.
    We will travel through %s, %s, and %s. Just passing through the systems should be sufficient. Also, I want to visit the %s station before returning back to %s. You have to make sure no one shoots us down during our expedition."]])
text[3] = _([["Please follow us, %s. Make sure to jump to the next system after we jumped out. We'll have to land on some planets on our way to refuel."]])
text[4] = _([[Suddenly your comm system turns on, receiving a conversation between the ship you are escorting and a Dvaered patrol ship.
    "I assure you, we mean no harm. We are just a convoy of scientists passing through Dvaered space." says Dr. Mensing.
    The response sounds harsh. "Do you think we're naïve? You're obviously a spy scouting our systems' defences."
    "Please calm down. I'm sure there is a diplomatic solution for our misunderstanding."
    The Dvaered officer replies "We can see that your ship is stuffed with sensors. Your intentions are obvious. Prepare for your ship being boarded."]])
text[5] = _([[Dr. Mensing  pauses, apparently choosing her words with care.
    "Fine, do whatever you want. Our reasoning is obviously beyond the imagination of your degenerate intellect." With this answer the comm shuts off. Your sensors show that a Dvaered patrol changed their course and is heading straight towards the transporter.]])
text[6] = _([["The situation would have escalated anyway." argues Dr. Mensing, this time directly speaking towards you.
    "I must admit, it is suspicious for a refitted transport ship with such advanced sensor suits to show up in Dvaered space. I haven't considered this point.
    I'm counting on you, %s. Please help us."]])
text[7] = _([[After leaving the ship you meet up with Dr. Mensing who hands you over a chip worth %s and thanks you for your help.
    "We'll be able to return to Jorla safely from here on. You did science a great favor today. I'm sure the data we collected will help us to understand the cause for the Sol nebula's volatility."]])

refueltitle = _("A short break")
refueltext = _([[Once you are done with the refuel operations, you meet Dr. Mensing on her way back to the transport ship.
    "I just met up with another 'scientist' working on this station. The purpose of this station is to collect data about the PSO nebula, but their scans are absolute garbage. Apparently the station is being run by an independent university. They couldn't possible keep up with the Za'lek standards in terms of proper scientific methods."
    She is visibly upset about the apparent lack of dedication to science. "Let's head back to %s. Our own measurements are completed by now."]])

transporterdeathtitle = _("The transporter was destroyed!")
transporterdeathtext = _("The transporter was destroyed and all scientists died! Even worse, you failed science!")
landfailtitle = _("You abandoned your mission!")
landfailtext = _("You have landed, abandoning your mission to escort the transport ship. You failed science miserably!")
transporterdistress = _("We're under attack!")

-- Mission info stuff
osd_msg   = {}
osd_title = _("Advanced Nebula Research")
osd_msg[1] = _("Escort the transport ship to %s in the %s system.")
osd_msg[2] = _("Land on %s in the %s system.")
osd_msg[3] = _("Fly back to %s in the %s system.")

log_text = _([[You helped Dr. Mensing to collect sensor data of the PSO nebula.]])


function create()
    station = "PSO Monitor"
    homeworld = "Bastion Station"
    ambush = false
    stage = 0
    credits = 800000  -- 800K
    
    -- Spaceport bar stuff
    misn.setNPC(_("A scientist"), "zalek/unique/mensing.png")
    misn.setDesc(bar_desc)
end

function accept()
    if not tk.yesno(title[1], string.format(text[1], player:name())) then
        misn.finish()
    end
    
    stage = 1
    exited = false
    firstTakeOff = true
    origin = planet.cur()
    t_sys = {}
    t_sys[1] = system.get("Ksher")
    t_sys[2] = system.get("Sultan")
    t_sys[3] = system.get("Faust")
    t_sys[4] = system.get("PSO")
    t_sys[5] = system.get("Amaroq")
    t_sys[6] = system.get("Ksher")
    t_sys[7] = system.get("Daravon")
    t_sys[8] = system.get("Pultatis")
    t_planet = {}
    t_planet[1] = planet.get("Qoman")
    t_planet[5] = planet.get(station)
    t_planet[6] = t_planet[1]
    t_planet[7] = planet.get("Vilati Vilata")
    t_planet[8] = homeworld
    
    tk.msg(title[1], string.format(text[2], t_sys[2]:name(), t_sys[3]:name(), t_sys[4]:name(), _(station), _(homeworld)))
    
    -- Set up mission information
    destsys = t_sys[1]
    misn.setTitle(mtitle)
    misn.setReward(string.format(misn_reward, creditstring(credits)))
    misn.setDesc(string.format(mdesc, _(station), t_sys[5]:name()))
    nextsys = getNextSystem(system.cur(), destsys) -- This variable holds the system the player is supposed to jump to NEXT.
    
    misn.accept()
    osd_msg[1] = string.format(osd_msg[1], t_planet[5]:name(), destsys:name())
    osd_msg[2] = string.format(osd_msg[2], _(station), t_sys[5]:name())
    osd_msg[3] = string.format(osd_msg[3], _(homeworld), t_sys[8]:name())
    misn.osdCreate(osd_title, osd_msg)
    misn_marker = misn.markerAdd(destsys, "low")
    
    hook.takeoff("takeoff")
    hook.jumpin("jumpin")
    hook.jumpout("jumpout")
    hook.land("land")
end

function takeoff()
    if firstTakeOff then
        tk.msg(title[2], string.format(text[3], player:name()))
        firstTakeOff = false
    end
    destplanet = nil
    spawnTransporter()
    misn.markerMove(misn_marker, destsys)
end

function jumpin()
     if system.cur() ~= nextsys then
        fail(_("MISSION FAILED! You jumped into the wrong system. You failed science miserably!"))
    else
        spawnTransporter()
        if not ambush and system.cur():faction() == faction.get("Dvaered") and system.cur():jumpDist(t_sys[5]) < 5 then
            hook.timer(2000, "startAmbush")
        elseif system.cur()==system.get("Daan") or system.cur()==system.get("Provectus Nova") then
            ambush = pilot.add("Trader Ambush 2", "baddie_norun", vec2.new(0,0))
            for i, j in ipairs(ambush) do
                j:setHostile()
                j:control(true)
                j:attack(transporter)
            end
        elseif system.cur() == t_sys[5] then
            misn.osdActive(2)
        end
    end
end

function jumpout()
    if not exited then
        fail(_("MISSION FAILED! You jumped before the transport ship you were escorting."))
    end
    origin = system.cur()
    nextsys = getNextSystem(system.cur(), destsys)
    if nextsys == t_sys[stage] then
        if t_planet[stage] ~= nil then
            destplanet = t_planet[stage]
        else
            destplanet = nil
        end
        stage = stage+1
        destsys = t_sys[stage]
        if destplanet ~= nil then
            misn.markerMove(misn_marker, destsys)
        end
    end
end

function land()
    if not exited then
        tk.msg(landfailtitle, landfailtext)
        misn.finish(false)
    elseif planet.cur() == planet.get(station) and not station_visited then
        tk.msg(refueltitle, string.format(refueltext, _(homeworld)))
        station_visited = true
        misn.osdActive(3)
    elseif planet.cur() == planet.get(homeworld) then
        tk.msg(title[5], string.format(text[7], creditstring(credits)))
        player.pay(credits)
        zlk_addNebuResearchLog(log_text)
        misn.finish(true)
    end
    origin = planet.cur()
end

function continueToDest(pilot)
    t_sys = {}
    t_sys[1] = system.get("Ksher")
    t_sys[2] = system.get("Sultan")
    t_sys[3] = system.get("Faust")
    t_sys[4] = system.get("PSO")
    t_sys[5] = system.get("Amaroq")
    t_sys[6] = system.get("Ksher")
    t_sys[7] = system.get("Daravon")
    t_sys[8] = system.get("Pultatis")
    t_planet = {}
    t_planet[1] = planet.get("Qoman")
    t_planet[5] = planet.get(station)
    t_planet[6] = t_planet[1]
    t_planet[7] = planet.get("Vilati Vilata")
    t_planet[8] = homeworld
    if pilot ~= nil and pilot:exists() then
        pilot:control(true)
        pilot:setNoJump(false)
        pilot:setNoLand(false)
        if destplanet ~= nil then
            pilot:land(destplanet, true)
        else
            pilot:hyperspace(getNextSystem(system.cur(), destsys), true)
        end
    end
end

function transporterJump(p, j)
    exited = true
    if p:exists() then
        player.msg(string.format(
            _("%s has jumped to %s."), p:name(), j:dest():name()))
    end
end

function transporterLand(p, j)
    exited = true
    if p:exists() then
        player.msg(string.format(
            _("%s has landed on %s."), p:name(), destplanet:name()))
    end
end

function transporterAttacked(p, attacker)
    unsafe = true
    p:control(true)
    p:setNoJump(true)
    p:setNoLand(true)
    p:attack(attacker)
    p:control(false)
    
    if not shuttingup then
        shuttingup = true
        p:comm(player.pilot(), transporterdistress)
        hook.timer(5000, "transporterShutup") -- Shuts him up for at least 5s.
    end
end

function transporterShutup()
     shuttingup = false
end

function timer_transporterSafe()
    hook.timer(2000, "timer_transporterSafe")
    
    if unsafe then
        unsafe = false
        continueToDest(transporter)
    end
end

function spawnTransporter()
    transporter = pilot.add("Nebula Research Shuttle", nil, origin)[1]
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
    hook.timer( 2000, "timer_transporterSafe" )
end

function startAmbush()
    ships = pilot.add("Dvaered Small Patrol", "dvaered_norun", vec2.new(-1000,0))
    for i, j in ipairs(ships) do
        j:control(true)
        j:moveto(vec2.new(-8000,0))
    end
    ambush = true
    hook.timer(15000, "ambushHail")
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
    tk.msg(title[3], text[4])
    tk.msg(title[3], text[5])
    tk.msg(title[3], string.format(text[6], player:name()))
end

-- Handle the destruction of the transporter. Abort the mission.
function transporterDeath()
    tk.msg(transporterdeathtitle, transporterdeathtext)
    misn.finish(false)
end

