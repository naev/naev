--[
-- This is the second mission in the "shadow" series.
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

    title = {}
    text = {}
    commmsg = {}
    
    title[1] = "Reunion with Rebina"
    text[1] = [[    You dock with the Seiryuu and shut down your engines. At the airlock, you are welcomed by two nondescript crewmen in grey uniforms who tell you to follow them into the ship. They lead you through corridors and passages that seem to lead to the bridge. On the way, you can't help but look around you in wonder. The ship isn't anything you're used to seeing. While some parts can be identified as such common features as doors and viewports, a lot of the equipment in the compartiments and niches seems strange, almost alien to you. Clearly the Seiryuu is not just any other Kestrel.
    On the bridge, you immediately spot - who else - the Seiryuu's captain, Rebina, seated in the captain's chair. The chair, too, is designed in the strange fashion that you've been seeing all over the ship. It sports several controls that you can't place, despite the fact that you're an experienced pilot yourself. The rest of the bridge is no different. All the regular stations and consoles seem to be there, but there are some others whose purpose you can only guess.
    Rebina swivels the chair around and smiles when she sees you. "Ah, %s," she says. "How good of you to come. I was hoping you'd get my invitation, since I was quite pleased with your performance last time. And I'm not the only one. As it turns out Jorek seems to have taken a liking to you as well. He may seem rough, but he's a good man at heart."]]
    
    text[2] = [[    You choose not to say anything, but Rebina seems to have no trouble reading what's on your mind. "Ah yes, the ship. It's understandable that you're surprised at how it looks. I can't divulge too much about this technology or how we came to possess it, but suffice to say that we don't buy from the regular outlets. We have need of... an edge in our line of business."
    Grateful for the opening, you ask Rebina what exactly this line of business is. Rebina flashes you a quick smile and settles into the chair for the explanation.
    "The organization I'm part of is known as the Four winds, or rather," she gestures dismissively, "not known as the Four Winds. We keep a low profile. You won't have heard of us before, I'm sure. At this point I should add that many who do know us refer to us as the 'Shadows', but this is purely a colloquial name. It doesn't cover what we do, certainly. In any event, you can think of us as a private operation with highly specific objectives. At this point that is all I can tell you." She leans forward and fixes you with a level stare. "Speaking of specific objectives, I have one such objective for you."]]
    
    textrepeat = [[    Again, you set foot on the Seiryuu's decks, and again you find yourself surrounded by the unfamiliar technology on board. The ship's crewmen guide you to the bridge, where Rebina is waiting for you. She says, "Welcome back, %s. I hope you've come to reconsider my offer. Let me explain to you again what it is we need from you."]]
    
    text[3] = [[    "You may not know this, but there are tensions between the Imperial and Dvaered militaries. For some time now there have been incidents on the border, conflicts about customs, pilots disrespecting each other's flight trajectories, that sort of thing. It hasn't become a public affair yet, and the respective authorities don't want it to come to that. This is why they've arranged a secret diplomatic meeting to smooth things over and make arrangements to de-escalate the situation.
    "This is where we come in. Without going into the details, suffice to say we have an interest in making sure that this meeting does not meet with any unfortunate accidents. However, for reasons I can't explain to you now, we can't become involved directly. That's why I want you to go on our behalf.
    "You will essentially be flying an escort mission. You will rendezvous with a small wing of private fighters, who will take you to your protegee, the Imperial representative. Once there, you will protect him from any threats you might encounter, and see him safely to Dvaered space. As soon as the Imperial representative has joined his Dvaered colleague, your mission will be complete and you will report back here.
    "That will be all. I offer you a suitable monetary reward should you choose to accept. Can I count on you to undertake this task?"]]
    
    refusetitle = "Let sleeping shadows lie"
    refusetext = [[    Captain Rebina sighs. "I see. I don't mind admitting that I hoped you would accept, but it's your decision. I won't force you to do anything you feel uncomfortable with. However, I still hold out the hope that you will change your mind. If you do, come back to see me. You know where to find the Seiryuu."
    
    Mere minutes later you find yourself back in your cockpit, and the Seiryuu is leaving. It doesn't really come as a surprise that you can't find any reference to your rendezvous with the Seiryuu in your flight logs...]]
    
    accepttitle = "Shadow Vigil"
    accepttext = [[    "Excellent, %s," Rebina smiles at you. "I've told my crew to provide your ship's computer with the necessary navigation data. Also, note that I've taken the liberty to install a specialized IFF transponder in your ship. Don't pay it any heed, it will only serve to identify you as one of the escorts. For various reasons, it is best you refrained from communication with the other escorts as much as possible. I think you might have an inkling as to why."
    Rebina straightens up. "That will be all for now, %s," she says in a more formal, captain-like manner. "You have your assignment I suggest you go about it."
    You are politely but efficiently escorted off the Seiryuu's bridge. Soon you settle back in your own cockpit chair, ready to do what was asked of you.]]
    
    wrongsystitle = "You diverged!"
    wrongsystext = [[You have jumped to the wrong system! You are no longer part of the mission to escort the diplomat.]]
    
    -- First meeting.
    commmsg[1] = "There you are at last. Fancy boat you've got there. Okay, you know the drill. Let's go."
    
    -- Enroute chatter.
    commmsg[2] = "So do you guys think we'll run into any trouble?"
    commmsg[3] = "Not if we all follow the plan. I didn't hear of any trouble coming our way from any of the others."
    commmsg[4] = "I just hope Z. knows what he's doing."
    commmsg[5] = "Cut the chatter, two, three. This is a low-profile operation. Act the part, please."
    
    -- Diplomat jumpin.
    commmsg[6] = "Alright boys, there he is. You know your orders. Stick to him, don't let anyone touch him on the way to the rendezvous."
    commmsg[7] = "Two, copy."
    commmsg[8] = "Three, copy."
    
    -- Enroute pirates.
    commmsg[9] = "Those rats are eyeballing us - take them out!"
    commmsg[10] = "All hostiles eliminated, resume standing orders."
    
    -- Endgame
    commmsg[11] = "This is Empire zero-zero-four. Transmitting clearance code now."
    commmsg[12] = "Empire zero-zero-four, your code checks out. Commende boarding maneuvers."
    commmsg[13] = "This is leader, you're all clear. Execute, execute, execute!"
    
    -- Mission info stuff
    osd_title = {}
    osd_msg   = {}
    osd_title = "Shadow Vigil"
    osd_msg[1] = "Fly to the %s system and join the other escorts"
    osd_msg[2] = "Follow the flight leader to the rendezvous location"
    osd_msg[3] = "Escort the Imperial diplomat"
    osd_msg[4] = "Report back to Rebina"
    
    misn_desc = [[Captain Rebina of the Four Winds has tasked you with an important mission.]]
    misn_reward = "A sum of money."
end

function create()
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
    sysname = "Tuoladis"
    destsys = system.get(sysname)
    misssys = {system.get("Qex"), system.get("Borla"), system.get("Doranthex")} -- Escort meeting point, protegee meeting point, final destination.
    misssys["__save"] = true
    nextsys = misssys[1]
    seirsys = system.cur()
    oldsys = system.cur()
    escorting = false
    chatter = false
    stage = 1
    
    first = var.peek("shadowvigil_first") == nil -- nil acts as true in this case.
    accepted = false
    missend = false

    var.push("shadowvigil_active", true)
    tk.msg(accepttitle, string.format(accepttext, player.name(), player.name()))

    misn.accept()
    
    misn.setDesc(misn_desc)
    misn.setReward(misn_reward)
    misn.setMarker(misssys[1], "misc")
    
    osd_msg[1] = string.format(osd_msg[1], misssys[1]:name())
    misn.osdCreate(osd_title, osd_msg)
    
    enterhook = hook.enter("enter")
    jumpinhook = hook.jumpin("jumpin")
    jumpouthook = hook.jumpin("jumpout")
    landhook = hook.land("land")
end

-- Function hooked to jumpout. Used to retain information about the previously visited system.
function jumpout()
    oldsys = system.cur()
end

-- Function hooked to landing. Only used to prevent a fringe case.
function land()
    if escorting and system.cur() == misssys[3] and stage == 3 then
        tk.msg("You landed in the final system!", "Fag.")
        abort()
    end
end

-- Function hooked to jumpin AND takeoff. Handles events that should occur in either case.
function enter()
    if system.cur() == misssys[1] and stage == 1 then
        -- case enter system where escorts wait
        escorts = pilot.add("Escort Lancelot Triplet", nil, vec2.new(0, 0))
        alive = {true, true, true} -- Keep track of the escorts. Update this when they die.
        for i, j in ipairs(escorts) do
            if j:exists() then
                j:rename(string.format("Four Winds Escort %d", i)) -- Note: not translate-friendly.
                j:control()
                hook.pilot(j, "death", "escortDeath")
            end
        end
        proxy1 = hook.timer(1000, "proximityOrigin")
    end
end

-- Function hooked to jumpin. Handles most of the events in the various systems.
function jumpin()
    if escorting then
        if system.cur() == misssys[2] then
            -- case join up with diplomat
            tk.msg("Implement me!", "Fag.")
            stage = 3 -- The actual escort begins here.
        elseif system.cur() == misssys[3] then
            -- case rendezvous with dvaered diplomat
        elseif not system.cur() == nextsys then
            -- case player is escorting AND jumped to somewhere other than the next escort destination
            tk.msg(wrongsystitle, wrongsystext)
            abort()
        else
            -- case enroute, handle escorts flying to the next system
            escorts = pilot.add("Escort Lancelot Triplet", nil, oldsys)
            for i, j in ipairs(escorts) do
                if not alive[i] then j:rm() end -- Dead escorts stay dead.
                if j:exists() then
                    j:rename(string.format("Four Winds Escort %d", i)) -- Note: not translate-friendly.
                    j:control()
                    hook.pilot(j, "death", "escortDeath")
                end
                j:hyperspace(getNextSystem(misssys[stage])) -- Hyperspace toward the next destination system.
            end
            if not chatter then
                hook.timer(10000, "chatter")
                hook.timer(15000, "chatter")
                hook.timer(20000, "chatter")
                hook.timer(25000, "chatter")
                chatter = true
            end
        end
    elseif system.cur() == seirsys then
        -- case enter system where Seiryuu is
        seiryuu = pilot.add("Seiryuu", nil, vec2.new(0, -2000))[1]
        seiryuu:setInvincible(true)
        if missend then
            seiryuu:disable()
            hook.pilot(seiryuu, "board", "board")
        end
    else
        if proxy1 then
            misn.timerStop(proxy1)
        end
    end
end

-- Poll for player proximity to the origin
function proximityOrigin()
    if vec2.dist(player.pos(), vec2.new(0, 0)) < 500 then
        escortStart()
    else
        proxy1 = hook.timer(1000, "proximityOrigin")
    end
end

-- The player has successfully joined up with the escort fleet. Cutscene -> departure.
function escortStart()
    escorting = true
    stage = 2 -- Fly to the diplomat rendezvous point
    escorts[1]:comm(commmsg[1])
    for i, j in pairs(escorts) do
        if j:exists() then
            j:hyperspace(getNextSystem(misssys[stage])) -- This is broken somehow.
        end
    end
end

-- State machine that handles the pilot conversation.
function chatter()
    if not (alive[1] and alive[2] and alive[3]) then
        return
    end
    -- TODO: state machine here.
end

-- Choose the next system to jump to on the route from the current system to the argument system.
function getNextSystem(finalsys)
    local mysys = system.cur()
    if mysys == finalsys then
        return mysys
    else
        neighs = mysys:adjacentSystems()
        local nearest = -1
        local nextsys = finalsys
        for j, _ in pairs(neighs) do
            if nearest == -1 or j:jumpDist(finalsys) < nearest then
                nearest = j:jumpDist(finalsys)
                nextsys = j
            end
        end
        return nextsys
    end
end

-- Handle the death of the escorts. Abort the mission if all escorts die.
function escortDeath()
    if alive[3] then alive[3] = false
    elseif alive[2] then alive[2] = false
    else -- all escorts dead
        -- TODO: abort message
        tk.msg("All the escorts am death!", "Fag.")
        abort()
    end
end

function board()
    player.unboard()
    seiryuu:setHealth(100,100)
    -- TODO: end mission text, reward
end

-- Handle the unsuccessful end of the mission.
function abort()
    var.pop("shadowvigil_active")
    misn.finish(false)
end
