--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Destroy the FLF base!">
 <unique />
 <priority>2</priority>
 <chance>10</chance>
 <location>Bar</location>
 <done>Dvaered Shopping</done>
 <cond>var.peek("flfbase_intro") == 3</cond>
 <faction>Dvaered</faction>
 <notes>
  <requires name="The Dvaered know where Sindbad is"/>
  <campaign>Doom the FLF</campaign>
  <provides name="The FLF is dead"/>
 </notes>
 <tags>
  <tag>dva_cap_ch01_lrg</tag>
 </tags>
</mission>
--]]
--[[
   This is the third mission in the anti-FLF Dvaered campaign. The player joins the battle to destroy the FLF base.
   stack variable flfbase_intro:
        1 - The player has turned in the FLF agent or rescued the Dvaered crew. Conditional for dv_antiflf02
        2 - The player has rescued the FLF agent. Conditional for flf_pre02
        3 - The player has found the FLF base for the Dvaered, or has betrayed the FLF after rescuing the agent. Conditional for dv_antiflf03
--]]

local fleet = require "fleet"
local fmt = require "format"
require "proximity"
local portrait = require "portrait"
local dv = require "common.dvaered"
local cinema = require "cinema"
local ai_setup = require "ai.core.setup"

-- Mission constants
local DVplanet, DVsys = spob.getS("Stalwart Citadel")
local basepos = spob.get("Sindbad"):pos()

local base, bomber, bombers, fighterpos, fightersDV, fleetDV, fleetFLF, fleetpos, obstinate, vendetta, vigilance -- Non-persistent state
local nextStage, spawnDV, spawnbase, updatepos -- Forward-declared functions

function create()
    local missys = {system.get(var.peek("flfbase_sysname"))}
    if not misn.claim(missys) then
        abort()
    end

    misn.setNPC(_("Dvaered liaison"), portrait.getMaleMil("Dvaered"), _("This must be the Dvaered liaison you heard about. Allegedly, he may have a job for you that involves fighting the Frontier Liberation Front."))
end

function accept()
    local first = not var.peek("flfbase_liaised")
    mem.destsys = system.get(var.peek("flfbase_sysname"))

    local briefing, txt
    briefing = fmt.f(_([[The liaison's expression then turns wooden, and his voice becomes level. Clearly, he has been briefing people for a long time in his career, so he can probably do this in his sleep. It occurs to you that perhaps he DOES nap while doing this.
    "In the near future, the Dvaered fleet will move against enemies of the state in the {sys} system. The objective is to seek out and destroy all hostiles. This operation will be headed by the HDSF Obstinate, and all units in this battle will defer to its commanding officer, regardless of class and rank. The Obstinate and its battle group will concentrate on performing bombing runs on the primary target. Your task as an auxiliary unit will be to secure the flanks and engage any hostiles that threaten the success of the mission. Note that once you enter the combat theatre, you are considered committed, and your leaving the system will be seen as an act of cowardice and treachery."
    The liaison blinks awake. "These are the parameters and conditions of the mission. Will you be accepting this assignment?"]]), {sys=mem.destsys})
    if first then
        txt = fmt.f(_([[The Dvaered liaison spots you, and stands up to shake your hand.
    "Well met, citizen {player}. I have heard about your recent achievements in the fight against the FLF threat. Like many Dvaered, I am pleased that things are going so well, and in no small way thanks to your efforts! High Command apparently feels the same way, because they have given you the military clearance for the upcoming operation, and that doesn't happen to just anybody."
    ]]), {player=player.name()}) .. briefing
    else
        txt = fmt.f(_([[The Dvaered liaison greets you.
    "I knew you'd be back, citizen {player}. The operation hasn't started yet and we can still use your help, so maybe I should explain to you again what this is all about."
    ]]), {player=player.name()}) .. briefing
        var.push("flfbase_liaised", true)
    end

    if tk.yesno(_("One swift stroke"), txt) then
        tk.msg(_("The battlefield awaits"), _([["Excellent. Please report to the local military command centre at 0400 today. You will be briefed there."
    The liaison hands you a small access card. It bears the emblem of the Dvaered military. It seems you've been granted a level of clearance that goes beyond that of a civilian volunteer.
    The liaison stands up, offers a curt greeting and walks out of the bar. You remain for a while, since you're not due for your briefing for some time yet. You reflect on your recent achievements. Your actions have drastically dipped the balance of power between the Dvaered and the FLF insurgents, and soon you will be able to see the results of your decisions with your own two eyes. You feel a sense of accomplishment to know you're making a difference in this galaxy.
    Several periods later, you find yourself in a functional, sterile briefing room at the Dvaered military base. You are joined by several Dvaered pilots, who are clearly going to be participating in the upcoming battle as well.]]))
        tk.msg(_("The battlefield awaits"), fmt.f(_([["Everyone," a stern-looking but otherwise nondescript Dvaered officer addresses the room, "If I may have your attention please. I am here to brief you on the upcoming operation, which as you all know revolves around the destruction of the terrorist base known as Sindbad."
    The wall behind the officer lights up, showing a schematic representation of the {sys} system. In the middle sits a red glowing disc with the FLF logo superimposed over it.
    "We're dealing with a fully operational, heavily armed military installation. The FLF may be terrorists, but they're well organized. Since this is their biggest stronghold, we must assume this base is heavily armed, and has a large complement of defensive spacecraft to defend it from attack. We will therefore conduct our assault in two phases."
    The system schematic on the wall updates with a cluster of white Dvaered logos, positioned some distance away from the glowing disc. There are also several white dots which apparently represent the fighter escorts.
    "Our strike force will consist out of the HDSF Obstinate, several destroyer escorts and two wings of fighter escorts. In addition, our forces will be joined by citizen {player}, who has volunteered to fight on behalf of House Dvaered on this occasion." The officer nods at you, then continues his briefing. "Our forces and formation will be such that it appears we are preparing for a standard strafing run, and indeed this is what will happen if the FLF decide to sit and cower. However, we expect them to put up a fight."]]), {sys=mem.destsys, player=player.name()}))
        tk.msg(_("The battlefield awaits"), fmt.f(_([[The schematic updates again, this time showing several small clusters of red dots between the glowing disc and the Dvaered formation.
   "The FLF will send out wings of fighters and bombers to engage our strike force. At this time the number and composition of ships is unknown, but it seems prudent to assume they will outnumber us by a fair margin. Your task as auxiliary escorts is to protect the strike force's flanks and intercept any FLF ships attempting to target the Obstinate. Be advised that the Obstinate will have limited anti-fighter armaments available, as most of its hull is dedicated to fighter bays. The Obstinate must not be destroyed! This is your paramount objective!"
    On the wall, the red dots are intercepted by the white dots, and blink out of existence. Then a second group of white dots appears near the Dvaered logos, and moves towards the glowing disc.
    "As soon as the FLF have exhausted their forces trying to counterattack, the HDSF Obstinate will begin launching bombers. It is our belief that the bombers alone will be able to take out the enemy base, but in the event that resistance is heavier than expected, the Obstinate herself will move in to provide fire support."
    The glowing disc on the wall fades out, leaving the Dvaered fleet alone and victorious.
    "That will be all. You have your orders. Report to your stations as per your timetables. I will see you all in {sys}. Good luck."
    Some time later, you are back in the Dvaered spaceport bar. You've seen action before in your career, but you still feel tense about what is to come. You decide to have another drink, just for luck.]]), {sys=mem.destsys}))

        misn.accept()
        misn.osdCreate(_("Destroy the FLF base!"), {
            fmt.f(_("Fly to the {sys} system"), {sys=mem.destsys}),
            _("Defend the HDSF Obstinate and its escorts"),
            _("Destroy the FLF base"),
            fmt.f(_("Return to {pnt} in the {sys} system"), {pnt=DVplanet, sys=DVsys}),
        })
        misn.setDesc(_("The Dvaered are poised to launch an all-out attack on the secret FLF base. You have chosen to join this battle for wealth and glory."))
        misn.setReward(_("Wealth and glory"))
        misn.setTitle(_("Destroy the FLF base!"))
        mem.mission_marker = misn.markerAdd( mem.destsys, "high" )

        mem.missionstarted = false
        mem.victorious = false

        hook.enter("enter")
        hook.land("land")
    else
        tk.msg(_("Refusal"), _([["Understood, citizen. Keep in mind, though, that as long as the operation isn't yet underway, you may still choose to participate. Simply come back to me if you change your mind."]]))
        return
    end
end

function enter()
    if system.cur() == mem.destsys and not mem.victorious then
        pilot.clear()
        pilot.toggleSpawn(false)

        fleetFLF = {}
        fleetDV = {}
        bombers = {}
        fleetpos = {}
        fightersDV = {}
        fighterpos = {}
        mem.DVbombers = 5 -- Amount of initial Dvaered bombers
        mem.DVreinforcements = 20 -- Amount of reinforcement Dvaered bombers
        mem.deathsFLF = 0
        mem.deathsFLFneeded = 0
        mem.timer = 0
        mem.stage = 0
        mem.standoff = 5000 -- The distance between the DV fleet and the base
        mem.safestandoff = 2000 -- This is the distance from the base where DV ships will turn back
        mem.stepsize = 180 / 10 -- The amount of points on the circle for the circular patrol
        mem.angle = math.pi * 1.5 - mem.stepsize

        spawnbase()
        spawnDV()
        obstinate:comm( _("All Dvaered vessels in position. Citizen, join the fleet and commence the attack!"), true )
        -- Wait for the player to fly to the Obstinate before commencing the mission

        hook.timer(0.5, "proximity", {anchor = obstinate, radius = 1500, funcname = "operationStart"})
    elseif system.cur() == DVsys and mem.victorious then -- Make sure the player can finish the missions properly.
        DVplanet:landOverride(true)
    elseif mem.missionstarted then -- The player has jumped away from the mission theater, which instantly ends the mission and with it, the mini-campaign.
        tk.msg(_("You ran away!"), _("You have left the system without first completing your mission. This treachery will not soon be forgotten by the Dvaered authorities!"))
        faction.get("Dvaered"):modPlayerSingle(-10)
        abort()
    end

end

function operationStart()
    misn.osdActive(2)
    mem.missionstarted = true
    mem.wavefirst = true
    mem.wavestarted = false
    mem.baseattack = false

    idle()
    hook.timer(10.0, "spawnFLFfighters")
    hook.timer(13.0, "spawnFLFfighters")
    hook.timer(15.0, "spawnFLFbombers")
    hook.timer(17.0, "spawnFLFfighters")
    mem.deathsFLFneeded = 11
    mem.controller = hook.timer(1.0, "control")
end

function land()
    if mem.victorious and spob.cur() == DVplanet then
        tk.msg(_("FLF base? What FLF base?"), fmt.f(_([[When you step out of your ship, a Dvaered military delegation is waiting for you. Normally this wouldn't be a good thing, as the Dvaered military typically see civilians as mobile patches of air, unless they've done something wrong. But this case is an exception. The soldiers politely escort you to the office of Colonel Urnus, the man who got you involved in this operation in the first place.
    "Well met, citizen {player}," Urnus tells you. "Cigar? Oh. Well, suit yourself. Anyway, I wanted to personally thank you for your role in the recent victory against the FLF. If it hadn't been for the information you provided we might have never smoked out their nest! In addition, your efforts on the battlefield have helped to secure our victory. House Dvaered recognizes accomplishments like that, citizen."
    The Colonel walks to a cabinet in his office and takes out a small box. From the box, he produces a couple of credit chips as well as a small metal pin in the shape of a star.
    "This is a reward for your services. The money speaks for itself, of course. As for the pin, it's a civilian commendation, the Star of Valor. Think of it as a badge of honor. It isn't a medal, but it's considered a mark of prestige among the Dvaered citizenry nonetheless. You will certainly enjoy greater respect when you wear this on your lapel, at least as long as you're in Dvaered space."]]), {player=player.name()}))
        tk.msg(_("FLF base? What FLF base?"), _([[Colonel Urnus returns to his seat.
    "Let me tell you one thing, though. I doubt we've quite seen the last of the FLF. We may have dealt them a mortal blow by taking out their hidden base, but as long as rebel sentiment runs high among the Frontier worlds, they will rear their ugly heads again. That means my job isn't over, and maybe it means yours isn't either. Perhaps in the future we'll work together again - but this time it won't be just about removing a threat on our doorstep." Urnus smiles grimly. "It will be about rooting out the source of the problem once and for all."
    As you walk the corridor that leads out of the military complex, the Star of Valor glinting on your lapel, you find yourself thinking about what your decisions might ultimately lead to. Colonel Urnus hinted at war on the Frontier, and he also indicated that you would be involved. While the Dvaered have been treating you as well as can be expected from a military regime, perhaps you might want to reconsider your allegiance when the time comes...]]))
        faction.get("Dvaered"):modPlayerSingle(10)
        player.pay(1e6)
        player.outfitAdd("Star of Valor")
        var.pop("flfbase_intro")
        var.pop("flfbase_liaised")
        var.pop("flfbase_sysname")
        diff.apply("flf_dead")
        dv.addAntiFLFLog( _([[You aided the Dvaered in the destruction of the secret FLF base, Sindbad. The terrorists are not entirely eliminated, but they have been substantially reduced in number. Colonel Urnus suggested you may be able to help the Dvaered again in a future campaign aimed at "rooting out the source of the problem once and for all".]]) )
        local t = time.get():tonumber()
        var.push( "invasion_time", t ) -- Timer for the frontier's invasion
        misn.finish(true)
    end
end

-- Spawns the FLF base, ship version.
function spawnbase()
    base = pilot.add( "Sindbad", "FLF", basepos, nil, {ai="flf_norun", naked=true} )
    base:outfitAdd("Base Ripper MK2", 8)
    base:setHostile()
    base:setNoDisable(true)
    base:setHilight(true)
    base:setVisplayer()
    hook.pilot(base, "death", "deathBase")
end

function deathBase()
    cinema.on()

    camera.set( base, false, 5000 )
    hook.timer( 8.0, "timer_plcontrol" )

    misn.osdActive(4)
    misn.markerMove( mem.mission_marker, DVsys )

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

    hook.rm(mem.controller)

    obstinate:comm( _("Terrorist station eliminated! All units, return to base."), true )
    obstinate:control()
    obstinate:hyperspace()
    obstinate:setHilight(false)
    obstinate:setNoDeath()

    mem.missionstarted = false
    mem.victorious = true
end

function timer_plcontrol ()
   cinema.off()
   camera.set( player.pilot(), false, 5000 )
end

-- Spawns the one-time-only Dvaered ships. Bombers are handled elsewhere.
function spawnDV()
    updatepos()

    obstinate = pilot.add( "Dvaered Goddard", "Dvaered", fleetpos[1], _("Obstinate"), {ai="dvaered_norun"} )
    obstinate:setDir(math.pi/2)
    obstinate:setFriendly()
    obstinate:setNoDisable(true)
    obstinate:control()
    obstinate:setHilight(true)
    obstinate:setVisplayer()
    obstinate:outfitRm("all")
    obstinate:outfitAdd("Engine Reroute")
    obstinate:outfitAdd("Small Shield Booster")
    obstinate:outfitAdd("Small Shield Booster")
    ai_setup.setup(obstinate)
    hook.pilot(obstinate, "attacked", "attackedObstinate")
    hook.pilot(obstinate, "death", "deathObstinate")
    hook.pilot(obstinate, "idle", "idle")

    -- Treat player as one more
    hook.pilot( player.pilot(), "attacked", "attacked" )

    for i = 1, 4 do
        vigilance = pilot.add( "Dvaered Vigilance", "Dvaered", fleetpos[i + 1], nil, {ai="dvaered_norun"} )
        vigilance:setDir(math.pi/2)
        vigilance:setFriendly()
        vigilance:control()
        vigilance:setVisplayer(true)
        hook.pilot(vigilance, "attacked", "attacked")
        fleetDV[#fleetDV + 1] = vigilance
    end

    for i = 1, 6 do
        vendetta = pilot.add( "Dvaered Vendetta", "Dvaered", fighterpos[i], nil, {ai="dvaered_norun"} )
        vendetta:setDir(math.pi/2)
        vendetta:setFriendly()
        vendetta:setVisplayer(true)
        vendetta:control()
        hook.pilot(vendetta, "attacked", "attacked")
        fightersDV[#fightersDV + 1] = vendetta
    end
end


-- Helper function
local function setFLF( j )
  hook.pilot(j, "death", "deathFLF")
  j:setNoDisable(true)
  j:setHostile()
  j:setVisible(true)
end


-- Spawns FLF fighters
function spawnFLFfighters()
    mem.wavefirst = true
    mem.wavestarted = true
    local wingFLF = fleet.add( 3, "Lancelot", "FLF", base:pos(), nil, {ai="flf_norun"} )
    for i, j in ipairs(wingFLF) do
        fleetFLF[#fleetFLF + 1] = j
        setFLF( j )
    end
end

-- Spawns FLF bombers
function spawnFLFbombers()
    local wingFLF = fleet.add( 3, "Vendetta", "FLF", base:pos(), nil, {ai="flf_norun"} )
    for i, j in ipairs(wingFLF) do
        fleetFLF[#fleetFLF + 1] = j
        setFLF( j )
    end
end

-- Spawns FLF destroyers
function spawnFLFdestroyers()
    local wingFLF = fleet.add( 2, "Pacifier", "FLF", base:pos(), nil, {ai="flf_norun"} )
    for i, j in ipairs(wingFLF) do
        fleetFLF[#fleetFLF + 1] = j
        hook.pilot(j, "death", "deathFLF")
        setFLF( j )
    end
end

local function pruneFLF()
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
    mem.deathsFLF = mem.deathsFLF + 1

    pruneFLF()

    -- Keep track of deaths
    if mem.deathsFLF >= mem.deathsFLFneeded then
        nextStage()
    end
end

-- Moves on to the next stage
function nextStage()
    if not mem.wavestarted then
       return
    end
    mem.wavestarted = false
    mem.timer = 0 -- Immediately recall the Dvaered escorts
    mem.stage = mem.stage + 1
    mem.deathsFLF = 0
    mem.deathsFLFneeded = 0
    if mem.stage <= 1 then
        --player.msg("Starting mem.stage 2.")
        hook.timer(1.0, "spawnFLFfighters")
        hook.timer(3.0, "spawnFLFbombers")
        hook.timer(5.0, "spawnFLFdestroyers")
        hook.timer(7.0, "spawnFLFbombers")
        mem.deathsFLFneeded = 10
    elseif mem.stage <= 2 then
        --player.msg("Starting stage 3.")
        hook.timer(1.0, "spawnFLFfighters")
        hook.timer(3.0, "spawnFLFdestroyers")
        hook.timer(5.0, "spawnFLFbombers")
        hook.timer(7.0, "spawnFLFbombers")
        hook.timer(9.0, "spawnFLFdestroyers")
        mem.deathsFLFneeded = 12
    elseif mem.stage <= 3 then
        --player.msg("Starting stage 4.")
        local delay = 0
        delay = delay + 3.0
        hook.timer(delay, "broadcast", {caster = obstinate, message = _("This is Obstinate. Launching bombers.")})
        hook.timer(delay, "spawnDVbomber")
        delay = delay + 38.0
        hook.timer(delay, "engageBase")
        delay = delay + 45.0
        hook.timer(delay, "destroyBase")
        misn.osdActive(3)
    else
        print(_("WARNING: dv_antiflf03: going to next stage, but next stage doesn't exist!"))
    end
end

-- Capsule function for pilot.broadcast, for timer use
function broadcast(args)
    args.caster:broadcast(args.message, true)
end

-- Spawns the initial Dvaered bombers.
function spawnDVbomber()
    bomber = pilot.add( "Dvaered Ancestor", "Dvaered", obstinate:pos(), nil, {ai="dvaered_norun"} )
    bomber:outfitRm("all")
    bomber:outfitAdd("TeraCom Imperator Launcher", 1, true, true)
    bomber:outfitAdd("Engine Reroute", 1)
    bomber:outfitAdd("Vulcan Gun", 3)
    ai_setup.setup(bomber)
    bomber:setNoDisable(true)
    bomber:setFriendly()
    bomber:control()
    bomber:attack(base)
    bombers[#bombers + 1] = bomber
    hook.pilot(bomber, "death", "deathDVbomber")
    mem.DVbombers = mem.DVbombers - 1
    if mem.DVbombers > 0 then
        mem.spawner = hook.timer(3.0, "spawnDVbomber")
    end
end

-- Makes remaining escorts engage the base
function engageBase()
    mem.baseattack = true

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
local function controlFleet( fleetCur, pos, off )
    if mem.baseattack then
       return
    end

    if fleetFLF ~= nil then
       pruneFLF()
    end

    -- Dvaered escorts should fall back into formation if not in combat, or if too close to the base or if too far from the Obstinate.
    for i, j in ipairs( fleetCur ) do
        if j:exists() then
            local basedist = vec2.dist(j:pos(), base:pos())
	    local nearest

            -- Get closest hostile
            local distance = 2500
            if fleetFLF ~= nil and #fleetFLF > 0 then
                nearest = nil
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
            if basedist < mem.safestandoff or mem.timer <= 0 then
                j:control()
                j:moveto( pos[i + off] )

            -- See if we should engage
            elseif nearest ~= nil and (distance < 500 or j:idle()) then
                j:control()
                j:attack(nearest)

            -- Fly back to fleet
            elseif j:idle() then
                j:control()
                j:moveto( pos[i + off] )
            end
        end
    end
end

-- Tries to whip the AI into behaving in a specific way
function control()
    -- Timer to have them return
    if mem.timer > 0 then
        mem.timer = mem.timer - 0.5
    end

    -- Control the fleets
    controlFleet( fleetDV, fleetpos, 1 )
    controlFleet( fightersDV, fighterpos, 0 )

    mem.controller = hook.timer(0.5, "control")
end

-- Replaces lost bombers. The supply is limited, though.
function deathDVbomber()
    if mem.DVreinforcements > 0 then
        mem.DVreinforcements = mem.DVreinforcements - 1
        for i, j in ipairs(bombers) do
            if not j:exists() then
                bomber = pilot.add( "Dvaered Ancestor", "Dvaered", obstinate:pos(), nil, {ai="dvaered_norun"} )
                bomber:outfitRm("all")
                bomber:outfitAdd("TeraCom Imperator Launcher", 1, true, true)
                bomber:outfitAdd("Engine Reroute", 1)
                bomber:outfitAdd("Vulcan Gun", 3)
                ai_setup.setup(bomber)
                bomber:setNoDisable(true)
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
    tk.msg(_("The flagship is dead!"), _("The HDSF Obstinate has been destroyed by the FLF defending forces! This mission can no longer be completed."))

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
    if mem.wavefirst then
        pilot.broadcast(obstinate, _("This is Obstinate, we're under fire!"), true)
        mem.wavefirst = false
    end
    attacked()
end

-- Make escorts fight back, then return to their positions
function attacked()
    mem.timer = 3.0

    for i, j in ipairs(fleetDV) do
        if j:exists() and (not base:exists() or vec2.dist(j:pos(), base:pos()) > mem.safestandoff) and vec2.dist(j:pos(), obstinate:pos()) < 1000 then
            j:control(false)
        end
    end

    for i, j in ipairs(fightersDV) do
        if j:exists() and (not base:exists() or vec2.dist(j:pos(), base:pos()) > mem.safestandoff) and vec2.dist(j:pos(), obstinate:pos()) < 1000 then
            j:control(false)
        end
    end
end

function idle()
    updatepos()
    obstinate:moveto(fleetpos[1], false, false)
end

function updatepos()
    mem.angle = math.fmod(mem.angle + mem.stepsize, 2*math.pi)
    fleetpos[1] = basepos + vec2.newP(mem.standoff, mem.angle)
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
