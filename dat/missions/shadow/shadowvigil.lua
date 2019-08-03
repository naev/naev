--[
-- This is the second mission in the "shadow" series.
--]]

include "proximity.lua"
include "dat/scripts/nextjump.lua"
include "chatter.lua"
include "selectiveclear.lua"

title = {}
text = {}
commmsg = {}

title[1] = _("Reunion with Rebina")
text[1] = _([[You dock with the Seiryuu and shut down your engines. At the airlock, you are welcomed by two nondescript crewmen in grey uniforms who tell you to follow them into the ship. They lead you through corridors and passages that seem to lead to the bridge. On the way, you can't help but look around you in wonder. The ship isn't anything you're used to seeing. While some parts can be identified as such common features as doors and viewports, a lot of the equipment in the compartments and niches seems strange, almost alien to you. Clearly the Seiryuu is not just any other Kestrel.
    On the bridge, you immediately spot - who else - the Seiryuu's captain, Rebina, seated in the captain's chair. The chair, too, is designed in the strange fashion that you've been seeing all over the ship. It sports several controls that you can't place, despite the fact that you're an experienced pilot yourself. The rest of the bridge is no different. All the regular stations and consoles seem to be there, but there are some others whose purpose you can only guess.
    Rebina swivels the chair around and smiles when she sees you. "Ah, %s," she says. "How good of you to come. I was hoping you'd get my invitation, since I was quite pleased with your performance last time. And I'm not the only one. As it turns out Jorek seems to have taken a liking to you as well. He may seem rough, but he's a good man at heart."]])
    
text[2] = _([[You choose not to say anything, but Rebina seems to have no trouble reading what's on your mind. "Ah yes, the ship. It's understandable that you're surprised at how it looks. I can't divulge too much about this technology or how we came to possess it, but suffice to say that we don't buy from the regular outlets. We have need of... an edge in our line of business."
    Grateful for the opening, you ask Rebina what exactly this line of business is. Rebina flashes you a quick smile and settles into the chair for the explanation.
    "The organization I'm part of is known as the Four Winds, or rather," she gestures dismissively, "not known as the Four Winds. We keep a low profile. You won't have heard of us before, I'm sure. At this point I should add that many who do know us refer to us as the 'Shadows', but this is purely a colloquial name. It doesn't cover what we do, certainly. In any event, you can think of us as a private operation with highly specific objectives. At this point that is all I can tell you." She leans forward and fixes you with a level stare. "Speaking of specific objectives, I have one such objective for you."]])
    
textrepeat = _([[Again, you set foot on the Seiryuu's decks, and again you find yourself surrounded by the unfamiliar technology on board. The ship's crewmen guide you to the bridge, where Rebina is waiting for you. She says, "Welcome back, %s. I hope you've come to reconsider my offer. Let me explain to you again what it is we need from you."]])
    
text[3] = _([["You may not know this, but there are tensions between the Imperial and Dvaered militaries. For some time now there have been incidents on the border, conflicts about customs, pilots disrespecting each other's flight trajectories, that sort of thing. It hasn't become a public affair yet, and the respective authorities don't want it to come to that. This is why they've arranged a secret diplomatic meeting to smooth things over and make arrangements to de-escalate the situation.
    "This is where we come in. Without going into the details, suffice to say we have an interest in making sure that this meeting does not meet with any unfortunate accidents. However, for reasons I can't explain to you now, we can't become involved directly. That's why I want you to go on our behalf.
    "You will essentially be flying an escort mission. You will rendezvous with a small wing of private fighters, who will take you to your charge, the Imperial representative. Once there, you will protect him from any threats you might encounter, and see him safely to Dvaered space. As soon as the Imperial representative has joined his Dvaered colleague, your mission will be complete and you will report back here.
    "That will be all. I offer you a suitable monetary reward should you choose to accept. Can I count on you to undertake this task?"]])
    
refusetitle = _("Let sleeping shadows lie")
refusetext = _([[Captain Rebina sighs. "I see. I don't mind admitting that I hoped you would accept, but it's your decision. I won't force you to do anything you feel uncomfortable with. However, I still hold out the hope that you will change your mind. If you do, come back to see me. You know where to find the Seiryuu."
    Mere minutes later you find yourself back in your cockpit, and the Seiryuu is leaving. It doesn't really come as a surprise that you can't find any reference to your rendezvous with the Seiryuu in your flight logs...]])
    
accepttitle = _("Shadow Vigil")
accepttext = _([["Excellent, %s," Rebina smiles at you. "I've told my crew to provide your ship's computer with the necessary navigation data. Also, note that I've taken the liberty to install a specialized IFF transponder in your ship. Don't pay it any heed, it will only serve to identify you as one of the escorts. For various reasons, it is best that you refrain from communication with the other escorts as much as possible. I think you might have an inkling as to why."
    Rebina straightens up. "That will be all for now, %s," she says in a more formal, captain-like manner. "You have your assignment; I suggest you go about it."
    You are politely but efficiently escorted off the Seiryuu's bridge. Soon you settle back in your own cockpit chair, ready to do what was asked of you.]])
    
title[4] = _("An unfortunate outcome")
text[4] = _([[Captain Rebina angrily drums her fingers on her captain's chair as she watches the reconstruction made from your sensor logs. Her eyes narrow when both diplomatic ships explode under the onslaught of weapons the escorts should not have had onboard.
    "This is bad, %s," she says when the replay shuts down. "Worse than I had even thought possible. The death of the Imperial and Dvaered diplomats is going to spark a political incident, with each faction accusing the other of treachery." She stands up and begins pacing up and down the Seiryuu's bridge. "But that's not the worst of it. You saw what happened. The diplomats were killed by their own escorts - by Four Winds operatives! This is an outrage!"
    Captain Rebina brings herself back under control through an effort of will. "%s, this does not bode well. We have a problem, and I fear I'm going to need your help again before the end. But not yet. I have a lot to do. I have to get to the bottom of this, and I have to try to keep this situation from escalating into a disaster. I will contact you again when I know more. In the mean time, you will have the time to spend your reward - it's already in your account."
    Following this, you are swiftly escorted off the Seiryuu. Back in your cockpit, you can't help feeling a little anxious about these Four Winds. Who are they, what do they want, and what is your role in all of it? Time will have to tell.]])
    
wrongsystitle = _("You diverged!")
wrongsystext = _([[You have jumped to the wrong system! You are no longer part of the mission to escort the diplomat.]])

escortdeathtitle = _("The escorts are dead!")
escortdeathtext = _([[All of the escorts have been destroyed. With the flight leader out of the picture, the diplomat has decided to call off the mission.]])

landfailtitle = _("You abandoned your charge!")
landfailtext = _("You have landed, but you were supposed to escort the diplomat. Your mission is a failure!")

diplomatdeathtitle = _("The diplomat is dead!")
diplomatdeathtext = _([[The diplomat you were supposed to be protecting has perished! Your mission has failed.]])

diplomatnoruntitle = _("You have left your charge behind!")
diplomatnoruntext = _([[You have jumped before the diplomat you were supposed to be protecting did. By doing so you have abandoned your duties, and failed your mission.]])

diplomatdistress = _("Diplomatic vessel under fire!")

-- First meeting.
commmsg[1] = _("There you are at last. Fancy boat you've got there. We're gonna head to Nova Shakar first, to grab some fuel. Just stick with us, okay?")

-- Enroute chatter.
commmsg[2] = _("So do you guys think we'll run into any trouble?")
commmsg[3] = _("Not if we all follow the plan. I didn't hear of any trouble coming our way from any of the others.")
commmsg[4] = _("I just hope Z. knows what he's doing.")
commmsg[5] = _("Cut the chatter, two, three. This is a low-profile operation. Act the part, please.")

-- Diplomat jumpin.
commmsg[6] = _("All right boys, there he is. You know your orders. Stick to him, don't let anyone touch him on the way to the rendezvous.")
commmsg[7] = _("Two, copy.")
commmsg[8] = _("Three, copy.")

-- Enroute pirates.
commmsg[9] = _("Those rats are eyeballing us - take them out!")
commmsg[10] = _("All hostiles eliminated, resume standing orders.")

-- Endgame
commmsg[11] = _("This is Empire zero-zero-four. Transmitting clearance code now.")
commmsg[12] = _("Empire zero-zero-four, your code checks out. Commence boarding maneuvers.")
commmsg[13] = _("This is your leader, you're all clear. Execute, execute, execute!")

-- Refuel hint
refueltitle = _("Preparing for the job")
refueltext = _("While you handle the post-land and refuel operations, you get a comm from the flight leader, audio only. He tells you that this will be the last place where you can refuel, and that you need to make sure to have at least %d jumps worth of fuel on board for the next leg of the journey. You will be left behind if you can't keep up.")

-- Mission info stuff
osd_title = {}
osd_msg   = {}
osd_title = _("Shadow Vigil")
osd_msg[1] = _("Fly to the %s system and join the other escorts")
osd_msg[2] = _("Follow the group to Nova Shakar and land")
osd_msg[3] = _("Follow the flight leader to the rendezvous location")
osd_msg[4] = _("Escort the Imperial diplomat")
osd_msg[5] = _("Report back to Rebina")

misn_desc = _([[Captain Rebina of the Four Winds has asked you to help Four Winds agents protect an Imperial diplomat.]])
misn_reward = _("A sum of money.")

function create()
    misssys = {system.get("Qex"), system.get("Shakar"), system.get("Borla"), system.get("Doranthex")} -- Escort meeting point, refual stop, protegee meeting point, final destination.
    misssys["__save"] = true
    
    if not misn.claim(misssys) then
        abort()
    end
    
    first = var.peek("shadowvigil_first") == nil -- nil acts as true in this case.
    if first then
        var.push("shadowvigil_first", false)
        tk.msg(title[1], string.format(text[1], player.name()))
        tk.msg(title[1], text[2])
        if tk.yesno(title[1], text[3]) then
            accept()
        else
            tk.msg(refusetitle, refusetext)
            abort()
        end
    else
        tk.msg(title[1], string.format(textrepeat, player.name()))
        if tk.yesno(title[1], text[3]) then
            accept()
        else
            tk.msg(refusetitle, refusetext)
            abort()
        end
    end
    player.unboard()
end

function accept()
    alive = {true, true, true} -- Keep track of the escorts. Update this when they die.
    alive["__save"] = true
    stage = 1 -- Keeps track of the mission stage
    nextsys = getNextSystem(system.cur(), misssys[stage]) -- This variable holds the system the player is supposed to jump to NEXT.
    seirsys = system.cur() -- Remember where the Seiryuu is.
    origin = system.cur() -- The place where the AI ships spawn from.
    chattered = false
    kills = 0 -- Counter to keep track of enemy kills.
    
    accepted = false
    missend = false
    landfail = false

    var.push("shadowvigil_active", true)
    tk.msg(accepttitle, string.format(accepttext, player.name(), player.name()))

    misn.accept()
    
    misn.setDesc(misn_desc)
    misn.setReward(misn_reward)
    marker = misn.markerAdd(misssys[1], "low")
    
    osd_msg[1] = string.format(osd_msg[1], misssys[1]:name())
    misn.osdCreate(osd_title, osd_msg)
    
    hook.land("land")
    hook.jumpin("jumpin")
    hook.enter("enter")
    hook.jumpout("jumpout")
    hook.takeoff("takeoff")
end

-- Function hooked to jumpout. Used to retain information about the previously visited system.
function jumpout()
    if stage == 4 and not dpjump then
        tk.msg(diplomatnoruntitle, diplomatnoruntext)
        abort()
    end
    origin = system.cur()
    nextsys = getNextSystem(system.cur(), misssys[stage])
end

-- Function hooked to landing. Only used to prevent a fringe case.
function land()
    if landfail then
        tk.msg(landfailtitle, landfailtext)
    elseif planet.cur() == planet.get("Nova Shakar") and stage == 2 then
        tk.msg(refueltitle, refueltext:format(system.cur():jumpDist(misssys[4])))
        stage = 3 -- Fly to the diplomat rendezvous point.
        misn.osdActive(3)
        landfail = true
        origin = planet.cur()
    end
end

-- Function hooked to takeoff.
function takeoff()
    if stage == 3 and landfail then -- We're taking off from Nova Shakar. Can't be anything else.
        jumpin()
    end
end

-- Function hooked to jumpin AND takeoff. Handles events that should occur in either case.
function enter()
    if system.cur() == misssys[1] and stage == 1 and missend == false then
        -- case enter system where escorts wait
        escorts = pilot.add("Shadowvigil Escorts", nil, vec2.new(0, 0))
        for i, j in ipairs(escorts) do
            if not alive[i] then j:rm() end -- Dead escorts stay dead.
            if j:exists() then
                j:control()
                j:setInvincPlayer()
                hook.pilot(j, "death", "escortDeath")
            end
        end
        rend_point = vec2.new(0,0)
        start_marker = system.mrkAdd( "Rendezvous point", rend_point )
        proxy = hook.timer(500, "proximity", {location = rend_point, radius = 500, funcname = "escortStart"})
    end
end

-- Function hooked to jumpin. Handles most of the events in the various systems.
function jumpin()
    sysclear = false -- We've just jumped in, so the ambushers, if any, are not dead.
    
    if stage >= 3 and system.cur() ~= nextsys then -- case player is escorting AND jumped to somewhere other than the next escort destination
        tk.msg(wrongsystitle, wrongsystext)
        abort()
    end
        
    if stage == 4 then
        -- Spawn the diplomat.
        diplomat = pilot.add("Shadowvigil Diplomat", nil, origin)[1]
        hook.pilot(diplomat, "death", "diplomatDeath")
        hook.pilot(diplomat, "jump", "diplomatJump")
        hook.pilot(diplomat, "attacked", "diplomatAttacked")
        diplomat:control()
        diplomat:setInvincPlayer()
        diplomat:setHilight(true)
        diplomat:setVisplayer()
        diplomat:setVisible() -- Hack to make ambushes more reliable.
        dpjump = false
        misn.markerRm(marker) -- No marker. Player has to follow the NPCs.
    end
    if stage >= 2 then
        pilot.toggleSpawn("Pirate")
        pilot.clearSelect("Pirate")

        -- Spawn the escorts.
        escorts = pilot.add("Shadowvigil Escorts", nil, origin)
        for i, j in ipairs(escorts) do
            if not alive[i] then j:rm() end -- Dead escorts stay dead.
            if j:exists() then
                j:control()
                j:setHilight(true)
                j:setInvincPlayer()
                hook.pilot(j, "death", "escortDeath")
                controlled = true
            end
        end

        -- Ships spawned, now decide what to do with them.
        if system.cur() == misssys[2] and stage == 2 then -- case land on Nova Shakar
           for i, j in ipairs(escorts) do
               if not alive[i] then j:rm() end -- Dead escorts stay dead.
               if j:exists() then
                  j:land(planet.get("Nova Shakar"))
               end
           end
        elseif system.cur() == misssys[3] then -- case join up with diplomat
            diplomat = pilot.add("Shadowvigil Diplomat", nil, vec2.new(0, 0))[1]
            hook.pilot(diplomat, "death", "diplomatDeath")
            hook.pilot(diplomat, "jump", "diplomatJump")
            diplomat:control()
            diplomat:setInvincPlayer()
            diplomat:setHilight(true)
            diplomat:setVisplayer()
            diplomat:setVisible() -- Hack to make ambushes more reliable.
            proxy = hook.timer(500, "proximity", {location = vec2.new(0, 0), radius = 500, funcname = "escortNext"})
            for i, j in ipairs(escorts) do
                if j:exists() then
                    j:follow(diplomat) -- Follow the diplomat.
                end
            end
            hook.timer(5000, "chatter", {pilot = escorts[1], text = commmsg[6]})
            hook.timer(12000, "chatter", {pilot = escorts[2], text = commmsg[7]})
            hook.timer(14000, "chatter", {pilot = escorts[3], text = commmsg[8]})
        elseif system.cur() == misssys[4] then -- case rendezvous with dvaered diplomat
            for i, j in ipairs(escorts) do
                if j:exists() then
                    j:follow(diplomat) -- Follow the diplomat.
                end
            end
            dvaerplomat = pilot.add("Dvaered Vigilance", nil, vec2.new(2000, 4000))[1]
            dvaerplomat:control()
            dvaerplomat:setHilight(true)
            dvaerplomat:setVisplayer()
            dvaerplomat:setDir(180)
            dvaerplomat:setFaction("Diplomatic")
            diplomat:setInvincible(true)
            diplomat:goto(vec2.new(1850, 4000), true)
            diplomatidle = hook.pilot(diplomat, "idle", "diplomatIdle")
        else -- case enroute, handle escorts flying to the next system, possibly combat
            for i, j in ipairs(escorts) do
                if j:exists() then
                    if stage == 4 then
                        diplomat:hyperspace(getNextSystem(system.cur(), misssys[stage])) -- Hyperspace toward the next destination system.
                        j:follow(diplomat) -- Follow the diplomat.
                    else
                        j:hyperspace(getNextSystem(system.cur(), misssys[stage])) -- Hyperspace toward the next destination system.
                    end
                end
            end
            if not chattered then
                hook.timer(10000, "chatter", {pilot = escorts[2], text = commmsg[2]})
                hook.timer(20000, "chatter", {pilot = escorts[3], text = commmsg[3]})
                hook.timer(30000, "chatter", {pilot = escorts[2], text = commmsg[4]})
                hook.timer(35000, "chatter", {pilot = escorts[1], text = commmsg[5]})
                chattered = true
            end
            if misssys[4]:jumpDist() <= 2 and misssys[4]:jumpDist() > 0 then -- Encounter
                ambush = pilot.add(string.format("Shadowvigil Ambush %i", 3 - misssys[4]:jumpDist()), "baddie_norun", vec2.new(0, 0))
                kills = 0
                for i, j in ipairs(ambush) do
                    if j:exists() then
                        j:setHilight(true)
                        hook.pilot(j, "death", "attackerDeath")

                        -- Just in case.
                        hook.pilot(j, "jump", "attackerDeath")
                        hook.pilot(j, "land", "attackerDeath")
                    end
                end
                for i, j in ipairs(escorts) do
                    if j:exists() then
                        hook.pilot(j, "attacked", "diplomatAttacked")
                    end
                end
            end
        end

    elseif system.cur() == seirsys then -- not escorting.
        -- case enter system where Seiryuu is
        seiryuu = pilot.add("Seiryuu", nil, vec2.new(0, -2000))[1]
        seiryuu:setInvincible(true)
        if missend then
            seiryuu:setActiveBoard(true)
            seiryuu:setHilight(true)
            seiryuu:setVisplayer(true)
            seiryuu:control()
            hook.pilot(seiryuu, "board", "board")
        end
    else
        if proxy then
            hook.rm( proxy )
        end
    end
end

-- The player has successfully joined up with the escort fleet. Cutscene -> departure.
function escortStart()
    if start_marker ~= nil then
       system.mrkRm( start_marker )
    end
    stage = 2 -- Fly to the refuel planet.
    misn.osdActive(2)
    misn.markerRm(marker) -- No marker. Player has to follow the NPCs.
    escorts[1]:comm(commmsg[1])
    for i, j in pairs(escorts) do
        if j:exists() then
            j:setHilight(true)
            j:hyperspace(getNextSystem(system.cur(), misssys[stage])) -- Hyperspace toward the next destination system.
        end
    end
end

-- The player has successfully rendezvoused with the diplomat. Now the real work begins.
function escortNext()
    stage = 4 -- The actual escort begins here.
    misn.osdActive(4)
    diplomat:hyperspace(getNextSystem(system.cur(), misssys[stage])) -- Hyperspace toward the next destination system.
    dpjump = false
end

-- Handle the death of the scripted attackers. Once they're dead, recall the escorts.
function attackerDeath()
    kills = kills + 1

    if kills < #ambush then return end

    local myj
    for i, j in ipairs(escorts) do
        if j:exists() then
            myj = j
            j:changeAI("baddie_norun")
            j:control()
            j:follow(diplomat)
            diplomat:hyperspace(getNextSystem(system.cur(), misssys[stage])) -- Hyperspace toward the next destination system.
            controlled = true
        end
    end
    
    myj:comm(commmsg[10])
    sysclear = true -- safety flag to prevent the escorts from being released twice.
end

-- Puts the escorts under AI control again, and makes them fight.
function escortFree()
    for i, j in ipairs(escorts) do
        if j:exists() then
            j:control(false)
            j:changeAI("baddie_norun")
        end
    end
end

-- Handle the death of the escorts. Abort the mission if all escorts die.
function escortDeath()
    if alive[3] then alive[3] = false
    elseif alive[2] then alive[2] = false
    else -- all escorts dead
        tk.msg(escortdeathtitle, escortdeathtext)
        abort()
    end
end

-- Handle the death of the diplomat. Abort the mission if the diplomat dies.
function diplomatDeath()
    -- TODO: abort message
    tk.msg(diplomatdeathtitle, diplomatdeathtext)
    for i, j in ipairs(escorts) do
        if j:exists() then
            j:control(false)
        end
    end
    abort()
end

-- Handle the departure of the diplomat. Escorts will follow.
function diplomatJump()
    dpjump = true
    misn.markerRm(marker)
    marker = misn.markerAdd(getNextSystem(system.cur(), misssys[stage]), "low")
    for i, j in ipairs(escorts) do
        if j:exists() then
            j:control(true)
            j:taskClear()
            j:hyperspace(getNextSystem(system.cur(), misssys[stage])) -- Hyperspace toward the next destination system.
        end
    end
    player.msg(string.format("Mission update: The diplomat has jumped to %s.", getNextSystem(system.cur(), misssys[stage]):name()))
end

-- Handle the diplomat getting attacked.
function diplomatAttacked()
    if controlled and not sysclear then
        chatter({pilot = escorts[1], text = commmsg[9]})
        escortFree()
        diplomat:taskClear()
        diplomat:brake()
        controlled = false
    end
    if shuttingup == true then return
    else
        shuttingup = true
        diplomat:comm(diplomatdistress)
        hook.timer(10000, "diplomatShutup") -- Shuts him up for at least 10s.
    end
end

-- As soon as the diplomat is at its destination, set up final cutscene.
function diplomatIdle()
    local mypos = {} -- Relative positions to the Dvaered diplomat.
    mypos[1] = vec2.new(0, 130)
    mypos[2] = vec2.new(-85, -65)
    mypos[3] = vec2.new(85, -65)
    
    for i, j in ipairs(escorts) do
        if j:exists() then
            j:setInvincible(true)
            j:taskClear()
            j:goto(dvaerplomat:pos() + mypos[i], true)
            j:face(dvaerplomat:pos())
        end
    end
    
    proxy = hook.timer(100, "proximity", {location = diplomat:pos(), radius = 400, funcname = "diplomatCutscene"})
end

-- This is the final cutscene.
function diplomatCutscene()
    player.pilot():control()
    player.pilot():brake()
    player.pilot():setInvincible(true)
    player.cinematics(true)
    
    camera.set(dvaerplomat, true, 500)
    
    hook.timer(1000, "chatter", {pilot = diplomat, text = commmsg[11]})
    hook.timer(10000, "chatter", {pilot = dvaerplomat, text = commmsg[12]})
    hook.timer(17000, "diplomatGo")
    hook.timer(21000, "chatter", {pilot = escorts[1], text = commmsg[13]})
    hook.timer(21500, "killDiplomats")
    
end

function diplomatShutup()
    shuttingup = false
end

function diplomatGo()
    diplomat:goto(dvaerplomat:pos(), true)
    hook.rm(diplomatidle)
end

function killDiplomats()
    for i, j in ipairs(escorts) do
        if j:exists() then
            j:taskClear()
            j:rmOutfit("all")
            j:addOutfit("Cheater's Ragnarok Beam", 1)
            j:attack(dvaerplomat)
            j:setHilight(false)
        end
    end
    diplomat:hookClear()
    hook.timer(500, "diplomatKilled")
    hook.timer(5000, "escortFlee")
    landfail = false
end

function diplomatKilled()
    diplomat:setHealth(0, 0)
    dvaerplomat:setHealth(0, 0)
end

function escortFlee()
    camera.set(player.pilot, true)

    player.pilot():setInvincible(false)
    player.pilot():control(false)
    player.cinematics(false)

    for i, j in ipairs(escorts) do
        if j:exists() then
            j:taskClear()
            j:rmOutfit("all")
            j:hyperspace()
        end
    end
    misn.osdActive(5)
    marker = misn.markerAdd(seirsys, "low")
    stage = 1 -- no longer spawn things
    missend = true
end

-- Function hooked to boarding. Only used on the Seiryuu.
function board()
    player.unboard()
    seiryuu:control()
    seiryuu:hyperspace()
    seiryuu:setActiveBoard(false)
    seiryuu:setHilight(false)
    tk.msg(title[4], string.format(text[4], player.name(), player.name()))
    player.pay(700000)
    var.pop("shadowvigil_active")
    misn.finish(true)
end

-- Handle the unsuccessful end of the mission.
function abort()
    var.pop("shadowvigil_active")
    misn.finish(false)
end
