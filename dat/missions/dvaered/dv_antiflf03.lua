--[[
-- This is the third mission in the anti-FLF Dvaered campaign. The player joins the battle to destroy the FLF base.
-- stack variable flfbase_intro:
--      1 - The player has turned in the FLF agent or rescued the Dvaered crew. Conditional for dv_antiflf02
--      2 - The player has rescued the FLF agent. Conditional for flf_pre02
--      3 - The player has found the FLF base for the Dvaered, or has betrayed the FLF after rescuing the agent. Conditional for dv_antiflf03
--]]

include("scripts/fleethelper.lua")
include("scripts/proximity.lua")

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title = {}
    text = {}
    failtitle = {}
    failtext = {}
    osd_desc = {}
    
    title[1] = "One swift stroke"
    introfirst = [[    The Dvaered liaison spots you, and stands up to shake your hand.
    "Well met, citizen %s. I have heard about your recent achievements in the fight against the FLF threat. Like many Dvaered, I am pleased that things are going so well, and in no small way thanks to your efforts! High Command apparently feels the same way, because they have given you the military clearance for the upcoming operation, and that doesn't happen to just anybody."
    ]]
    
    introrepeat = [[    The Dvaered liaison greets you.
    "I knew you'd be back, citizen %s. The operation hasn't started yet and we can still use your help, so maybe I should explain to you again what this is all about."
    ]]
    
    text[1] = [[The liaison's expression then turns wooden, and his voice becomes level. Clearly, he has been briefing people for a long time in his career, so he can probably do this in his sleep. It occurs to you that perhaps he DOES nap while doing this.
    "In the near future, the Dvaered fleet will move against enemies of the state in the %s system. The objective is to seek out and destroy all hostiles. This operation will be headed by the HDSF Obstinate, and all units in this battle will defer to its commanding officer, regardless of class and rank. The Obstinate and its battle group will concentrate on performing bombing runs on the primary target. Your task as an auxiliary unit will be to secure the flanks and engage any hostiles that threaten the success of the mission. Note that once you enter the combat theater, you are considered committed, and your leaving the system will be seen as an act of cowardice and treachery."
    The liaison blinks awake. "These are the parameters and conditions of the mission. Will you be accepting this assignment?"]]
    
    title[2] = "The battlefield awaits"
    text[2] = [[    "Excellent. Please report to the local military command center at 0400 today. You will be briefed there."
    The liaison hands you a small access card. It bears the logo of the Dvaered military. It seems you've been granted a level of clearance that goes beyond that of a civilian volunteer.
    The liaison stands up, offers a curt greeting and walks out of the bar. You remain for a while, since you're not due for your briefing for some time yet. You reflect on your recent achievements. Your actions have drastically dipped the balance of power between the Dvaered and the FLF insurgents, and soon you will be able to see the results of your decisions with your own two eyes. You feel a sense of accomplishment to know you're making a difference in this galaxy.
    
    Several hours later, you find yourself in a functional, sterile briefing room at the Dvaered military base. You are joined by several Dvaered pilots, who are clearly going to be participating in the upcoming battle as well.]]
    
    text[3] = [["Gentlemen," a stern-looking but otherwise nondescript Dvaered officer addresses the room, "If I may have your attention please. I am here to brief you on the upcoming operation, which as you all know revolves around the destruction of the terrorist base known as Sindbad."
    The wall behind the officer lights up, showing a schematic representation of the %s system. In the middle sits a red glowing disc with the FLF logo superimposed over it.
    "We're dealing with a fully operational, heavily armed military installation. The FLF may be terrorists, but they're well organized. Since this is their biggest stronghold, we must assume this base is heavily armed, and has a large complement of defensive spacecraft to defend it from attack. We will therefore conduct our assault in two phases."
    The system schematic on the wall updates with a cluster of white Dvaered logos, positioned some distance away from the glowing disc. There are also several white dots which apparently represent the fighter escorts.
    "Our strike force will consist out of the HDSF Obstinate, several destroyer escorts and two wings of fighter escorts. In addition, our forces will be joined by citizen %s, who has volunteered to fight on behalf of House Dvaered on this occasion." The officer nods at you, then continues his briefing. "Our forces and formation will be such that it appears we are preparing for a standard strafing run, and indeed this is what will happen if the FLF decide to sit and cower. However, we expect them to put up a fight."]]
    
    text[4] = [[The schematic updates again, this time showing several small clusters of red dots between the glowing disc and the Dvaered formation.
    "The FLF will send out wings of fighters and bombers to engage our strike force. At this time the number and composition of ships is unknown, but it seems prudent to assume they will outnumber us by a fair margin. Your task as auxiliary escorts is to protect the strike force's flanks and intercept any FLF ships attempting to target the Obstinate. Be advised that the Obstinate will have limited anti-fighter armaments available, as most of its hull is dedicated to fighter bays. The Obstinate must not be destroyed! This is your paradigm objective!"
    On the wall, the red dots are intercepted by the white dots, and blink out of existence. Then a second group of white dots appears near the Dvaered logos, and moves towards the glowing disc.
    "As soon as the FLF have exhausted their forces trying to counterattack, the HDSF Obstinate will begin launching bombers. It is our belief that the bombers alone will be able to take out the enemy base, but in the event that resistance is heavier than expected, the Obstinate herself will move in to provide fire support."
    The glowing disc on the wal fades out, leaving the Dvaered fleet alone and victorious.
    "That will be all. You have your orders. Report to your stations as per your timetables. I will see you all in %s. Good luck."
    
    Some time later, you are back in the Dvaered spaceport bar. You've seen action before in your career, but you still feel tense about what is to come. You decide to have another drink, just for luck.]]
    
    title[3] = "FLF base? What FLF base?"
    text[5] = [[    When you step out of your ship, a Dvaered military delegation is waiting for you. Normally this wouldn't be a good thing, as the Dvaered military typically see civilians as mobile patches of air, unless they've done something wrong. But this case is an exception. The soldiers politely escort you to the office of Colonel Urnus, the man who got you involved in this operation in the first place.
    "Well met, citizen %s," Urnus tells you. "Cigar? Oh. Well, suit yourself. Anyway, I wanted to personally thank you for your role in the recent victory against the FLF. If it hadn't been for the information you provided we might have never smoked out their nest! In addition, your efforts on the battlefield have helped to secure our victory. House Dvaered recognizes accomplishments like that, citizen."
    The Colonel walks to a cabinet in his office and takes out a small box. From the box, he produces a couple of credit chips as well as a small metal pin in the shape of a star.
    "This is a reward for your services. The money speaks for itself, of course. As for the pin, it's a civilian commendation, the Star of Valor. Think of it as a badge of honor. It isn't a medal, but it's considered a mark of prestige among the Dvaered citizenry nonetheless. You will certainly enjoy greater respect when you wear this on your lapel, at least as long as you're in Dvaered space."]]
    
    text[6] = [[    Colonel Urnus returns to his seat.
    "Let me tell you one thing, though. I doubt we've quite seen the last of the FLF. We may have dealt them a mortal blow by taking out their hidden base, but as long as rebel sentiment runs high among the Frontier worlds, they will rear their ugly heads again. That means my job isn't over, and maybe it means yours isn't either. Perhaps in the future we'll work together again - but this time it won't be just about removing a threat on our doorstep." Urnus smiles grimly. "It will be about rooting out the source of the problem once and for all."
    
    As you walk the corridor that leads out of the military complex, the Star of Valor glinting on your lapel, you find yourself thinking about what your decisions might ultimately lead to. Colonel Urnus hinted at war on the Frontier, and he also indicated that you would be involved. While the Dvaered have been treating you as well as can be expected from a military regime, perhaps you might want to reconsider your allegiance when the time comes...]]
    
    refusetitle = "Refusal"
    refusetext = [[    "Understood, citizen. Keep in mind, though, that as long as the operation isn't yet underway, you may still choose to participate. Simply come back to me if you change your mind."]]
    
    failtitle[1] = "You ran away!"
    failtext[1] = "You have left the system without first completing your mission. This treachery will not soon be forgotten by the Dvaered authorities!"
    
    failtitle[2] = "The flagship is dead!"
    failtext[2] = "The HDSF Obstinate has been destroyed by the FLF defending forces! This mission can no longer be completed."
    
    flagattack = "This is Obstinate, we're under fire!"
    phasetwo = "This is Obstinate. Launching bombers."
    
    npc_desc = "This must be the Dvaered liaison you heard about. Allegedly, he may have a job to you that involves fighting the Frontier Liberation Front."
    
    misn_title = "Destroy the FLF base!"
    osd_desc[1] = "Fly to the %s system"
    osd_desc[2] = "Defend the HDSF Obstinate and its escorts"
    osd_desc[3] = "Destroy the FLF base"
    osd_desc[4] = "Return to %s in the %s system"
    
    misn_desc = "The Dvaered are poised to launch an all-out attack on the secret FLF base. You have chosen to join this battle for wealth and glory."
end

function create()
    missys = {system.get(var.peek("flfbase_sysname"))}
    if not misn.claim(missys) then
        abort()
    end 

    misn.setNPC("Dvaered liaison", "dv_liason")
    misn.setDesc(npc_desc)
end

function accept()
    destsysname = var.peek("flfbase_sysname")
    DVplanet = "Stalwart Station"
    DVsys = "Darkstone"
    
    if first then
        txt = string.format(introfirst, player.name()) .. string.format(text[1], destsysname)
    else
        txt = string.format(introrepeat, player.name()) .. string.format(text[1], destsysname)
    end
    
    if tk.yesno(title[1], txt) then
        tk.msg(title[2], string.format(text[2], destsysname))
        tk.msg(title[2], string.format(text[3], destsysname, player.name()))
        tk.msg(title[2], string.format(text[4], destsysname))

        misn.accept()
        osd_desc[1] = string.format(osd_desc[1], destsysname)
        osd_desc[4] = string.format(osd_desc[4], DVplanet, DVsys)
        misn.osdCreate(misn_title, osd_desc)
        misn.setDesc(misn_desc)
        misn.setReward("Wealth, glory")
        misn.setTitle(misn_title)
        mission_marker = misn.markerAdd( system.get(destsysname), "high" )
        
        missionstarted = false
        victorious = false
        
        hook.enter("enter")
        hook.land("land")
    else
        tk.msg(refusetitle, refusetext)
        misn.finish()
    end
end

function enter()
    if system.cur():name() == destsysname and not victorious then
        pilot.clear()
        pilot.toggleSpawn(false)

        fleetFLF = {}
        fleetDV = {}
        bombers = {}
        fleetpos = {}
        fightersDV = {}
        fighterpos = {}
        basepos = vec2.new(-8700, -3000) -- NOTE: Should be the same coordinates as in asset.xml!
        DVbombers = 5 -- Amount of initial Dvaered bombers
        DVreinforcements = 20 -- Amount of reinforcement Dvaered bombers
        deathsFLF = 0
        time = 0
        stage = 0
        standoff = 5000 -- The distance between the DV fleet and the base
        safestandoff = 2000 -- This is the distance from the base where DV ships will turn back
        stepsize = math.pi / 10 -- The amount of points on the circle for the circular patrol
        angle = math.pi * 1.5 - stepsize
    
        spawnbase()
        spawnDV()
        -- Wait for the player to fly to the Obstinate before commencing the mission

        hook.timer(500, "proximity", {anchor = obstinate, radius = 1500, funcname = "operationStart"})
    elseif missionstarted then -- The player has jumped away from the mission theater, which instantly ends the mission and with it, the mini-campaign.
        tk.msg(failtitle[1], failtext[1])
        faction.get("Dvaered"):modPlayerRaw(-10)
        abort()
    end
    
end

function operationStart()
    misn.osdActive(2)
    missionstarted = true
    wavefirst = true
    wavestarted = false
    baseattack = false
    
    idle()
    hook.timer(10000, "spawnFLFfighters")
    hook.timer(15000, "spawnFLFfighters")
    tim_sec = hook.timer(100000, "nextStage")
    controller = hook.timer(1000, "control")
end

function land()
    if victorious and planet.cur() == planet.get("Darkstone") then
        tk.msg(title[3], string.format(text[5], player.name()))
        tk.msg(title[3], text[6])
        faction.get("Dvaered"):modPlayerRaw(10)
        player.pay(100000) -- 100K
        var.pop("flfbase_intro")
        var.pop("flfbase_sysname")
        misn.finish(true)
    end
end

-- Spawns the FLF base, ship version.
function spawnbase()
    base = pilot.add("FLF Base", "flf_norun", basepos)[1]
    base:rmOutfit("all")
    base:addOutfit("Base Ripper MK2", 8)
    base:setHostile()
    base:setNodisable(true)
    base:setHilight(true)
    base:setVisplayer()
    hook.pilot(base, "death", "deathBase")
end

function deathBase()
    misn.osdActive(4)
    misn.markerMove( mission_marker, system.get(DVsys) )

    for i, j in ipairs(bombers) do
        if j:exists() then
            j:control()
            j:hyperspace()
        end
    end

    for i, j in ipairs(fleetDV) do
        if j:exists() then
            j:control()
            j:hyperspace()
        end
    end

    for i, j in ipairs(fightersDV) do
        if j:exists() then
            j:control()
            j:hyperspace()
        end
    end
    
    hook.rm(controller)
   
    obstinate:control()
    obstinate:hyperspace()
    obstinate:setHilight(false)
    
    hook.timer(12000, "zoomTo")
    hook.timer(12000, "playerControl", false)

    missionstarted = false
    victorious = true
end

-- Spawns the one-time-only Dvaered ships. Bombers are handled elsewhere.
function spawnDV()
    updatepos()

    obstinate = pilot.add("Dvaered Goddard", "dvaered_norun", fleetpos[1])[1]
    obstinate:rename("Obstinate")
    obstinate:setDir(90)
    obstinate:setFriendly()
    obstinate:setNodisable(true)
    obstinate:control()
    obstinate:setHilight(true)
    obstinate:setVisible()
    obstinate:rmOutfit("all")
    obstinate:addOutfit("Engine Reroute")
    obstinate:addOutfit("Shield Booster")
    obstinate:addOutfit("Shield Booster")
    hook.pilot(obstinate, "attacked", "attackedObstinate")
    hook.pilot(obstinate, "death", "deathObstinate")
    hook.pilot(obstinate, "idle", "idle")
    
    local i = 1
    while i <= 4 do
        vigilance = pilot.add("Dvaered Vigilance", "dvaered_norun", fleetpos[i + 1])[1]
        vigilance:setDir(90)
        vigilance:setFriendly()
        vigilance:control()
        vigilance:setVisible(true)
        hook.pilot(vigilance, "attacked", "attacked")
        hook.pilot(vigilance, "death", "deathDV")
        fleetDV[#fleetDV + 1] = vigilance
        i = i + 1
    end
    
    local i = 1
    while i <= 6 do
        vendetta = pilot.add("Dvaered Vendetta", "dvaered_norun", fighterpos[i])[1]
        vendetta:setDir(90)
        vendetta:setFriendly()
        vendetta:setVisible(true)
        vendetta:control()
        hook.pilot(vendetta, "attacked", "attacked")
        hook.pilot(vendetta, "death", "deathDV")
        fightersDV[#fightersDV + 1] = vendetta
        i = i + 1
    end
end

-- Gets an array of possible Dvaered targets for the FLF to attack
function possibleDVtargets()
    targets = {}
    -- Bias towards escorts, twice as likely as obstinate or player
    for i, j in ipairs(fleetDV) do
        if j:exists() then
            targets[#targets + 1] = j
        end
    end
    for i, j in ipairs(fleetDV) do
        if j:exists() then
            targets[#targets + 1] = j
        end
    end
    -- Player and obstinate get added seperately
    targets[#targets + 1] = player.pilot()
    targets[#targets + 1] = obstinate
    return targets
end

-- Spawns FLF fighters
function spawnFLFfighters()
    wavefirst = true
    wavestarted = true
    local targets = possibleDVtargets()
    wingFLF = addShips( "FLF Vendetta", "flf_norun", base:pos(), 3 )
    for i, j in ipairs(wingFLF) do
        fleetFLF[#fleetFLF + 1] = j
        hook.pilot(j, "death", "deathFLF")
        j:setNodisable(true)
        j:setVisible(true)
        j:control()
        j:attack(targets[rnd.rnd(#targets - 1) + 1])
    end
end

-- Spawns FLF bombers
function spawnFLFbombers()
    local targets = possibleDVtargets()
    wingFLF = addRawShips( "Ancestor", "flf_norun", base:pos(), "FLF", 3 )
    for i, j in ipairs(wingFLF) do
        fleetFLF[#fleetFLF + 1] = j
        hook.pilot(j, "death", "deathFLF")
        j:setNodisable(true)
        j:setVisible(true)
        j:control()
        j:attack(targets[rnd.rnd(#targets - 1) + 1])
    end
end

-- An FLF ship just died
function deathFLF()
    deathsFLF = deathsFLF + 1

    -- Remove dead ships from array
    local t = fleetFLF
    fleetFLF = {}
    for i, j in ipairs(t) do
        if j:exists() then
            fleetFLF[ #fleetFLF+1 ] = j
        end
    end

    -- Keep track of deaths
    if #fleetFLF <= 0 then
        nextStage()
    end
end

-- Moves on to the next stage
function nextStage()
    if not wavestarted then
       return
    end
    wavestarted = false
    time = 0 -- Immediately recall the Dvaered escorts
    stage = stage + 1
    deathsFLF = 0
    hook.rm( tim_sec ) -- Stop security timer
    if stage == 1 then
        player.msg("Starting stage 2.")
        hook.timer(1000, "spawnFLFbombers")
        hook.timer(5000, "spawnFLFfighters")
        tim_sec = hook.timer(90000, "nextStage")
    elseif stage == 2 then
        player.msg("Starting stage 3.")
        hook.timer(1000, "spawnFLFfighters")
        hook.timer(3000, "spawnFLFbombers")
        hook.timer(5000, "spawnFLFbombers")
        tim_sec = hook.timer(90000, "nextStage")
    else
        player.msg("Starting stage 4.")
        local delay = 0
        hook.timer(delay, "playerControl", true)
        hook.timer(delay, "zoomTo", obstinate)
        delay = delay + 3000
        hook.timer(delay, "broadcast", {caster = obstinate, text = phasetwo})
        hook.timer(delay, "spawnDVbomber")
        delay = delay + 7000
        hook.timer(delay, "zoomTo", base)
        delay = delay + 50000
        hook.timer(delay, "engagebase")
        misn.osdActive(3)
    end
end

-- Capsule function for camera.set, for timer use
function zoomTo(target)
    camera.set(target, true, zoomspeed)
end

-- Capsule function for pilot.broadcast, for timer use
function broadcast(args)
    args.caster:broadcast(args.text, true)
end

-- Capsule function for player.pilot():control(), for timer use
-- Also saves the player's velocity.
function playerControl(status)
    player.pilot():control(status)
    player.cinematics(status)
    if status then
        pvel = player.pilot():vel()
        player.pilot():setVel(vec2.new(0, 0))
    else
        player.pilot():setVel(pvel)
    end
end 
 
-- Spawns the initial Dvaered bombers.
function spawnDVbomber()
    bomber = pilot.add("Dvaered Ancestor", "dvaered_norun", obstinate:pos())[1]
    bomber:rmOutfit("all")
    bomber:addOutfit("Imperator Launcher", 1)
    bomber:addOutfit("Engine Reroute", 1)
    bomber:addOutfit("Laser Cannon MK1", 3)
    bomber:setNodisable(true)
    bomber:setFriendly()
    bomber:control()
    bomber:attack(base)
    bombers[#bombers + 1] = bomber
    hook.pilot(bomber, "death", "deathDVbomber")
    DVbombers = DVbombers - 1
    if DVbombers > 0 then
        spawner = hook.timer(3000, "spawnDVbomber")
    end
end 

-- Makes remaining escorts engage the base
function engageBase()
    baseattack = true

    for i, j in ipairs(fleetDV) do
        if j:exists() and base:exists() then
            j:control()
            j:attack(base)
        end
    end

    for i, j in ipairs(fightersDV) do
        if j:exists() and base:exists() then
            j:control()
            j:attack(base)
        end
    end
end


-- Controls a fleet
function controlFleet( fleetCur, pos, off )
    -- Dvaered escorts should fall back into formation if not in combat, or if too close to the base or if too far from the Obstinate.
    for i, j in ipairs( fleetCur ) do
        if j:exists() then
            local attacking = false

            -- Kill nearby hostiles
            if fleetFLF ~= nil and #fleetFLF > 0 and j:idle() then
                local distance = 1500
                local nearest = nil
                local distanceCur
                for k, v in ipairs(fleetFLF) do
                    if v:exists() then
                        distanceCur = vec2.dist(j:pos(), v:pos())
                        if distanceCur < distance then
                            nearest = v
                            distance = distanceCur
                        end
                    end
                end
                if nearest ~= nil then
                    j:control()
                    j:attack(nearest)
                    attacking = true
                end
            end

            -- Fly back to fleet
            if ((not attacking or vec2.dist(j:pos(), base:pos()) < safestandoff or time <= 0 or j:idle()) and not baseattack) then
                j:control()
                j:goto( pos[i + off] )
            end
        end
    end
end

-- Tries to whip the AI into behaving in a specific way
function control()
    -- Timer to have them return
    if time > 0 then
        time = time - 500
    end
   
    -- Control the fleets
    controlFleet( fleetDV, fleetpos, 1 )
    controlFleet( fightersDV, fighterpos, 0 )
    
    controller = hook.timer(500, "control")
end

-- Replaces lost bombers. The supply is limited, though.
function deathDVbomber()
    if DVreinforcements > 0 then
        DVreinforcements = DVreinforcements - 1
        for i, j in ipairs(bombers) do
            if not j:exists() then
                bomber = pilot.add("Dvaered Ancestor", "dvaered_norun", obstinate:pos())[1]
                bomber:rmOutfit("all")
                bomber:addOutfit("Imperator Launcher", 1)
                bomber:addOutfit("Engine Reroute", 1)
                bomber:addOutfit("Laser Cannon MK1", 3)
                bomber:setNodisable(true)
                bomber:setFriendly()
                bomber:control()
                bomber:attack(base)
                hook.pilot(bomber, "death", "deathDVbomber")
                bombers[i] = bomber
            end
        end
    end
end

-- Handle mission failure.
function deathObstinate()
    tk.msg(failtitle[2], failtext[2])

    for i, j in ipairs(fleetDV) do
        if j:exists() then
            j:control()
            j:hyperspace()
        end
    end

    for i, j in ipairs(fightersDV) do
        if j:exists() then
            j:control()
            j:hyperspace()
        end
    end

    base:setHilight(false)

    abort()
end

function attackedObstinate()
    if wavefirst then
        pilot.broadcast(obstinate, flagattack, true)
        wavefirst = false
    end
    attacked()
end

-- Make escorts fight back, then return to their positions
function attacked()
    time = 3000
    
    for i, j in ipairs(fleetDV) do
        if j:exists() and vec2.dist(j:pos(), base:pos()) > safestandoff and vec2.dist(obstinate:pos(), obstinate:pos()) < 1000 then
            j:control(false)
        end
    end
    
    for i, j in ipairs(fightersDV) do
        if j:exists() and vec2.dist(j:pos(), base:pos()) > safestandoff and vec2.dist(obstinate:pos(), obstinate:pos()) < 1000 then
            j:control(false)
        end
    end
end

-- Re-target the FLF units when a Dvaered ship dies
function deathDV()
    local targets = possibleDVtargets()

    for i, j in ipairs(fleetFLF) do
        if j:exists() then
            j:attack(targets[rnd.rnd(#targets - 1) + 1])
        end
    end
end

function idle()
    updatepos()
    obstinate:goto(fleetpos[1], false, false)
end

function updatepos()
    angle = (angle + stepsize) % (2 * math.pi)
    fleetpos[1] = basepos + vec2.new(math.cos(angle) * standoff, math.sin(angle) * standoff)
    fleetpos[2] = fleetpos[1] + vec2.new(110, 0)
    fleetpos[3] = fleetpos[1] + vec2.new(-110, 0)
    fleetpos[4] = fleetpos[1] + vec2.new(0, -120)
    fleetpos[5] = fleetpos[1] + vec2.new(0, 120)
    fighterpos[1] = fleetpos[2] + vec2.new(60, 0)
    fighterpos[2] = fleetpos[2] + vec2.new(100, 0)
    fighterpos[3] = fleetpos[2] + vec2.new(80, -50)
    fighterpos[4] = fleetpos[3] + vec2.new(-60, 0)
    fighterpos[5] = fleetpos[3] + vec2.new(-100, 0)
    fighterpos[6] = fleetpos[3] + vec2.new(-80, -50)
end

function abort()
    misn.finish(false)
end
