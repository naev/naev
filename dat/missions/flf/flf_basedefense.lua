--[[
-- This is the flf version of dv_antiflf03.  Instead of attacking the flf base, you will defend it.  Reuses code from dv_antiflf03
-- To Do (anyone): improve dialog, add flf dialog
--]]

include "fleethelper.lua" 
include "proximity.lua"

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title = {}
    text = {}
    failtitle = {}
    failtext = {}
    osd_desc = {}
    
    title[1] = "Trouble is Coming"
    
    text[1] = [[    "Bad news, %s: the Dvaered figured out the location of our secret base.  They are already on their way there with a task force large enough to take out our base.  We are already preparing our base defenses, but we could use some additional help.  Are you interested?"]]
    
    title[2] = "Head to %s"
    text[2] = [[    "Great, get your best ship and biggest guns and head over to %s."]]

    title[3] = "The battlefield awaits"
    text[3] = [[As you land, Corporal Benito comes hurrying up to you.
    "The Dvaered strike has already jumped into the system and is heading this way.  Since our launchpad space is limited, you will be part of the 1st wave of the 3rd defense group.  Good luck out there."]]
    
    text[4] = [[Shockwaves and debris from exploding ships rock the base as you prepare your ship for the upcoming battle.  You watch the base crew scurry from ship to ship loading amunition and fuel.  Their faces are grim.  You can tell the battle isn't going well.]]
    
    title[4] = "Congratulations"
    text[5] = [[    When you step out of your ship, you can see everyone already celebrating the great victory.  Corporal Benito comes up to you and presses a credit chip into your hand.
    "Its not much, but we couldn't have won without your help."]]
    
    refusetitle = "Refusal"
    refusetext = [[    "I can understand why you wouldn't want to face the Dvaered.  We have sent out some forces to launch diversionary attacks in Dvaered space which may force them to pull back their strike force."]]
    
    failtitle[1] = "You ran away!"
    failtext[1] = "You have left the system.  Sindbad was destroyed!  (Please reload your save game before landing to try again."
 
    failtitle[2] = "Boom!"
    failtext[2] = "Sindbad was destroyed!  (Please reload your save game before landing to try again."
     
    passtitle = "The flagship is dead!"
    passtext = "The HDSF Obstinate has been destroyed!"
    
    flagattack = "This is Obstinate, we're under fire!"
    phasetwo = "This is Obstinate. Launching bombers."
    
    npc_desc = "You see an FLF officer heading your way."
    
    misn_title = "Defend the FLF base!"
    osd_desc[1] = "Fly to the %s system and land on %s"
    osd_desc[2] = "Defeat the HDSF Obstinate and its escorts before they destory %s"
    osd_desc[3] = "Land on %s"
    
    misn_desc = "Defend %s from the Dvaered strike force."
end

function create()
    missys = {system.get("Sigur")}
    if not misn.claim(missys) then
        abort()
    end 

    misn.setNPC("FLF petty officer", "flf/unique/benito")
    misn.setDesc(npc_desc)
end

function accept()
    destsysname = "Sigur"
    destplanet = "Sindbad"
    txt = string.format(text[1], player.name())
    
    if tk.yesno(title[1], txt) then
        tk.msg(string.format(title[2],destsysname), string.format(text[2], destsysname))
        misn.accept()
        osd_desc[1] = string.format(osd_desc[1], destsysname, destplanet)
        osd_desc[2] = string.format(osd_desc[2], destplanet)
        osd_desc[3] = string.format(osd_desc[3], destplanet)
	misn_desc = string.format(misn_desc, destplanet)
        misn.osdCreate(misn_title, osd_desc)
        misn.setDesc(misn_desc)
        misn.setReward("Saving the FLF base")
        misn.setTitle(misn_title)
        mission_marker = misn.markerAdd( system.get(destsysname), "high" )
        
        missionstarted = false
        victorious = false
        
        hook.land("land")
    else
        tk.msg(refusetitle, refusetext)
        misn.finish()
    end
end

function takeoff()
    if system.cur():name() == destsysname and not victorious then
        pilot.clear()
        pilot.toggleSpawn(false)
	diff.remove("FLF_base")
        fleetFLF = {}
        fleetDV = {}
        bombers = {}
        fleetpos = {}
        fightersDV = {}
        fighterpos = {}
        basepos = vec2.new(-8700, -3000) -- NOTE: Should be the same coordinates as in asset.xml!
        DVbombers = 5 -- Amount of initial Dvaered bombers
        DVreinforcements = 30 -- Amount of reinforcement Dvaered bombers
        deathsFLF = 0
        time = 0
        stage = 0
        standoff = 5000 -- The distance between the DV fleet and the base
        safestandoff = 2000 -- This is the distance from the base where DV ships will turn back
        stepsize = math.pi / 10 -- The amount of points on the circle for the circular patrol
        angle = math.pi * 1.5 - stepsize
    
        spawnbase()
        spawnDV()
        obstinate:comm( "All Dvaered vessels continue the attack!", true )

        hook.timer(500, "operationStart")
	hook.jumpin("jumped")
    end
    
end

function jumped()
    if missionstarted then -- The player has jumped away from the mission theater, which instantly ends the mission and with it, the mini-campaign.
        tk.msg(failtitle[1], failtext[1])
	var.pop("flfbase_intro")
        var.pop("flfbase_sysname")
        faction.get("FLF"):modPlayerSingle(-10)
        misn.finish(true)
    end
end

function operationStart()
    misn.osdActive(2)
    missionstarted = true
    wavefirst = true
    wavestarted = false
    baseattack = false
    
    idle()
    hook.timer(1000, "spawnFLFfighters")
    hook.timer(3000, "spawnFLFfighters")
    hook.timer(5000, "spawnFLFbombers")
    hook.timer(7000, "spawnFLFfighters")
    tim_sec = hook.timer(20000, "nextStage")
    controller = hook.timer(1000, "control")
end

function land()
    if victorious and planet.cur():name() == "Sindbad" then
        tk.msg(title[4], string.format(text[5], player.name()))
        faction.get("FLF"):modPlayerSingle(10)
        player.pay(50000) -- 50K
        misn.finish(true)
    elseif planet.cur():name() == "Sindbad" then
        tk.msg(title[3], string.format(text[3]))  
        tk.msg(title[3], string.format(text[4]))  
        misn.osdActive(2)
        hook.takeoff("takeoff") 
        var.pop("flfbase_sysname")
    end
end

-- Spawns the FLF base, ship version.
function spawnbase()
    base = pilot.add("FLF Base", "flf_norun", basepos)[1]
    base:rmOutfit("all")
    base:addOutfit("Base Ripper MK2", 8)
    base:setNodisable(true)
    base:setHilight(true)
    base:setVisplayer()
    hook.pilot(base, "death", "deathBase")
end

function deathBase()
    tk.msg(failtitle[2], failtext[2])
    var.pop("flfbase_intro")
    var.pop("flfbase_sysname")
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

    obstinate:comm( "Terrorist station eliminated! All units, return to base.", true )
    obstinate:control()
    obstinate:hyperspace()
    obstinate:setHilight(false)
    
    missionstarted = false
    victorious = true
    misn.finish(true)
end

-- Spawns the one-time-only Dvaered ships. Bombers are handled elsewhere.
function spawnDV()
    updatepos()

    obstinate = pilot.add("Dvaered Goddard", "dvaered_norun", fleetpos[1])[1]
    obstinate:rename("Obstinate")
    obstinate:setDir(90)
    obstinate:setHostile()
    obstinate:setNodisable(true)
    obstinate:control()
    obstinate:setHilight(true)
    obstinate:setVisible()
    obstinate:rmOutfit("all")
    obstinate:addOutfit("Engine Reroute")
    obstinate:addOutfit("Medium Shield Booster")
    obstinate:addOutfit("Medium Shield Booster")
    hook.pilot(obstinate, "attacked", "attackedObstinate")
    hook.pilot(obstinate, "death", "deathObstinate")
    hook.pilot(obstinate, "idle", "idle")

    local i = 1
    while i <= 4 do
        vigilance = pilot.add("Dvaered Vigilance", "dvaered_norun", fleetpos[i + 1])[1]
        vigilance:setDir(90)
        vigilance:setHostile()
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
        vendetta:setHostile()
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
    targets[#targets + 1] = obstinate
    return targets
end


-- Helper function
function setFLF( j )
  hook.pilot(j, "death", "deathFLF")
  j:setNodisable(true)
  j:setFriendly()
  j:setVisible(true)
  j:control()
end


-- Spawns FLF fighters
function spawnFLFfighters()
    wavefirst = true
    wavestarted = true
    local targets = possibleDVtargets()
    local wingFLF = addShips( "FLF Vendetta", "flf_norun", base:pos(), 3 )
    for i, j in ipairs(wingFLF) do
        fleetFLF[#fleetFLF + 1] = j
        setFLF( j )
        j:setVisible(true)
        j:attack(targets[rnd.rnd(#targets - 1) + 1])
    end
end

-- Spawns FLF bombers
function spawnFLFbombers()
    local targets = possibleDVtargets()
    local wingFLF = addRawShips( "Ancestor", "flf_norun", base:pos(), "FLF", 3 )
    for i, j in ipairs(wingFLF) do
        fleetFLF[#fleetFLF + 1] = j
        setFLF( j )
        hook.pilot(j, "death", "deathFLF")
        j:rename("FLF Ancestor")
        j:setVisible(true)
        j:attack(targets[rnd.rnd(#targets - 1) + 1])
    end
end

-- Spawns FLF destroyers
function spawnFLFdestroyers()
    local targets = possibleDVtargets()
    local wingFLF = addShips( "FLF Pacifier", "flf_norun", base:pos(), 2 )
    for i, j in ipairs(wingFLF) do
        fleetFLF[#fleetFLF + 1] = j
        hook.pilot(j, "death", "deathFLF")
        setFLF( j )
        j:setVisible(true)
        j:attack(targets[rnd.rnd(#targets - 1) + 1])
    end
end

function pruneFLF()
    -- Remove dead ships from array
    local t = fleetFLF
    fleetFLF = {}
    for i, j in ipairs(t) do
        if j:exists() then
            fleetFLF[ #fleetFLF+1 ] = j
        end
    end
end

-- An FLF ship just died
function deathFLF()
    deathsFLF = deathsFLF + 1

    pruneFLF()

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
        hook.timer(1000, "spawnFLFfighters")
        hook.timer(3000, "spawnFLFbombers")
        hook.timer(5000, "spawnFLFdestroyers")
        hook.timer(7000, "spawnFLFbombers")
        tim_sec = hook.timer(20000, "nextStage")
    elseif stage == 2 then
        hook.timer(1000, "spawnFLFfighters")
        hook.timer(3000, "spawnFLFdestroyers")
        hook.timer(5000, "spawnFLFbombers")
        hook.timer(7000, "spawnFLFbombers")
        hook.timer(9000, "spawnFLFdestroyers")
        tim_sec = hook.timer(30000, "nextStage")
    else
        local delay = 0
        delay = delay + 3000
        temp1 = hook.timer(delay, "broadcast", {caster = obstinate, text = phasetwo})
        temp2 = hook.timer(delay, "spawnDVbomber")
        delay = delay + 38000
        temp3 = hook.timer(delay, "engageBase")
        delay = delay + 45000
        temp4 = hook.timer(delay, "destroyBase")
    end
end

-- Capsule function for pilot.broadcast, for timer use
function broadcast(args)
    args.caster:broadcast(args.text, true)
end

 
-- Spawns the initial Dvaered bombers.
function spawnDVbomber()
    bomber = pilot.add("Dvaered Ancestor", "dvaered_norun", obstinate:pos())[1]
    bomber:rmOutfit("all")
    foo = bomber:addOutfit("TeraCom Imperator Launcher", 1, true)
    bomber:addOutfit("Engine Reroute", 1)
    bomber:addOutfit("Vulcan Gun", 3)
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


function destroyBase()
   if base:exists() then
      base:setHealth( -1, 0 )
   end
end


-- Controls a fleet
function controlFleet( fleetCur, pos, off )
    if baseattack then
       return
    end

    if fleetFLF ~= nil then
       pruneFLF()
    end

    -- Dvaered escorts should fall back into formation if not in combat, or if too close to the base or if too far from the Obstinate.
    for i, j in ipairs( fleetCur ) do
        if j:exists() then
            local basedist = vec2.dist(j:pos(), base:pos())

            -- Get closest hostile
            local distance = 2500
            if fleetFLF ~= nil and #fleetFLF > 0 then
                local nearest = nil
                -- Should all exist as we've pruned before
                for k, v in ipairs(fleetFLF) do
                    local distanceCur = vec2.dist(j:pos(), v:pos())
                    if distanceCur < distance then
                        nearest = v
                        distance = distanceCur
                    end
                end
            end

            -- Too close to base or recalled
            if basedist < safestandoff or time <= 0 then
                j:control()
                j:goto( pos[i + off] )

            -- See if we should engage
            elseif nearest ~= nil and (distance < 500 or j:idle()) then
                j:control()
                j:attack(nearest)

            -- Fly back to fleet
            elseif j:idle() then
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
    
    if DVreinforcements > 0 and obstinate:exists() then
        DVreinforcements = DVreinforcements - 1
        for i, j in ipairs(bombers) do
            if not j:exists() then
                bomber = pilot.add("Dvaered Ancestor", "dvaered_norun", obstinate:pos())[1]
                bomber:rmOutfit("all")
                bomber:addOutfit("TeraCom Imperator Launcher", 1, true)
                bomber:addOutfit("Engine Reroute", 1)
                bomber:addOutfit("Vulcan Gun", 3)
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

-- Handle mission success.
function deathObstinate()
    tk.msg(passtitle, passtext)
    misn.osdActive(3)
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
    diff.apply("FLF_base")
    missionstarted = false
    victorious = true
    if tim_sec ~= nil then
        hook.rm(tim_sec)
    end
    if temp1 ~= nil then
        hook.rm(temp1)
    end
    if temp2 ~= nil then
        hook.rm(temp2)
    end
    if temp3 ~= nil then
        hook.rm(temp3)
    end
    if temp4 ~= nil then
        hook.rm(temp4)
    end
    hook.rm(controller)
    for i, j in ipairs(fleetFLF) do
        if j:exists() then
            j:control()
            j:land(planet.get(destplanet))
        end
    end
    base:rm()
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
        if j:exists() and vec2.dist(j:pos(), base:pos()) > safestandoff and vec2.dist(j:pos(), obstinate:pos()) < 1000 then
            j:control(false)
        end
    end
    
    for i, j in ipairs(fightersDV) do
        if j:exists() and vec2.dist(j:pos(), base:pos()) > safestandoff and vec2.dist(j:pos(), obstinate:pos()) < 1000 then
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
    fleetpos[1] = basepos - vec2.new(math.cos(angle) * standoff, math.sin(angle) * standoff)
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
