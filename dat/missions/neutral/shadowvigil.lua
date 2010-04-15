--[
-- This is the second mission in the "shadow" series.
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

    title = {}
    text = {}
    
    title[1] = "Reunion with Rebina"
    text[1] = [[    You dock with the Seiryuu and shut down your engines. At the airlock, you are welcomed by two nondescript crewmen in grey uniforms who tell you to follow them into the ship. They lead you through corridors and passages that seem to lead to the bridge. On the way, you can't help but look around you in wonder. The ship isn't anything you're used to seeing. While some parts can be identified as such common features as doors and viewports, a lot of the equipment in the compartiments and niches seems strange, almost alien to you. Clearly the Seiryuu is not just any other Kestrel.
    On the bridge, you immediately spot - who else - the Seiryuu's captain, Rebina, seated in the captain's chair. The chair, too, is designed in the strange fashion that you've been seeing all over the ship. It sports several controls that you can't place, despite being an experienced pilot yourself. The rest of the bridge is no different. All the regular stations and consoles seem to be there, but there are some others whose purpose you can only guess.
    Rebina swivels the chair around and smiles when she sees you. "Ah, %," she says. "How good of you to come. I was hoping you'd get my invitation, since I was quite pleased with your performance last time. And I'm not the only one. As it turns out Jorek seems to have taken a liking to you as well. He may seem rough, but he's a good man at heart."]]
    
    text[2] = [[    You choose not to say anything, but Rebina seems to have no trouble reading what's on your mind. "Ah yes, the ship. It's understandable that you're surprised at how it looks. I can't divulge too much about this technology or how we came to possess it, but suffice to say that we don't buy from the regular outlets. We have need of... an edge in our line of business."
    Grateful for the opening, you ask Rebina what exactly this line of business is. Rebine flashes you a quick smile and settles into the chair for the explanation.
    "The organization I'm part of is known as the Four winds, or rather," she gestures dismissively, "not known as the Four Winds. We keep a low profile. You won't have heard of us before, I'm sure. At this point I should add that many who do know us refer to us as the 'Shadows', but this is purely a colloquial name. It doesn't cover what we do, certainly. In any event, you can think of us as a private operation with highly specific objectives. At this point that is all I can tell you." She leans forward and fixes you with a level stare. "Speaking of specific objectives, I have one such objective for you."]]
    
    textrepeat = [[    Again, you set foot on the Seiryuu's decks, and again you find yourself surrounded by the unfamiliar technology on board. The ship's crewmen guide you to the bridge, where Rebina is waiting for you. She says, "Welcome back, %s. I hope you've come to reconsider my offer. Let me explain to you again what it is we need from you."]]
    
    text[3] = [[    "You may not know this, but there are tensions between the Imperial and Dvaered militaries. For some time now there have been incidents on the border, conflicts about customs, pilots disrespecting each other's flight trajectories, that sort of thing. It hasn't become a public affair yet, and the respective authorities don't want it to come to that. This is why they've arranged a secret diplomatic meeting to smooth things over and make arrangements to de-escalate the situation.
    "This is where we come in. Without going into the details, suffice to say we have an interest in making sure that this meeting does not meet with any unfortunate accidents. However, for reasons I can't explain to you now, we can't become involved directly. That's why I want you to go on our behalf.
    "You will essentially be flying an escort mission. You will rendezvous with a small wing of private fighters, who will take you to your protegee, the Imperial representative. Once there, you will protect him from any threats you might encounter, and see him safely to Dvaered space. As soon as the Imperial representative has joined his Dvaered colleague, your mission will be complete and you will report back here.
    "That will be all. Can I count on you to undertake this task?"]]
    
    refusetitle = "Let sleeping shadows lie"
    refusetext = [[    Captain Rebina sighs. "I see. I don't mind admitting that I hoped you would accept, but it's your decision. I won't force you to do anything you feel uncomfortable with. However, I still hold out the hope that you will change your mind. If you do, come back to see me. You know where to find the Seiryuu."
    
    Mere minutes later you find yourself back in your cockpit, and the Seiryuu is leaving. It doesn't really come as a surprise that you can't find any reference to your rendezvous with the Seiryuu in your flight logs...]]
    
    accepttitle = ""
    accepttext = [[]]
    
    -- OSD stuff
    osd_title = {}
    osd_msg   = {}
    osd_title = "Shadow Vigil"
    osd_msg[1] = "Fly to the %s system and wait for the others to arrive"
    osd_msg[2] = "Follow the flight leader to the rendezvous location"
    osd_msg[3] = "Escort the Imperial diplomat"
    osd_msg[4] = "Report back to Rebina"
    
    misn_desc = [[Captain Rebina of the Four Winds has tasked you with an important mission.]]
    misn_reward = "A sum of money."
end

function create()
    sysname = "Tuoladis"
    destsys = system.get(sysname)
    misssys = {system.get("Qex"), system.get("Borla"), system.get("Doranthex")} -- Escort meeting point, protegee meeting point, final destination.
    seirsys = system.cur()
    
    first = var.peek("shadowvigil_first") == true -- Make sure it's not nil
    accepted = false
    missend = false

    if first then
        tk.msg(title[1], string.format(text[1], player.name()))
        tk.msg(title[1], text[2])
        if tk.yesno(title[1], text[3]) then
            accept()
        else
            tk.msg(refusetitle, refusetext)
            abort()
        end
        first = false
    else
        tk.msg(title[1], string.format(textrepeat, player.name()))
        if tk.yesno(title[2], text[3]) then
            accept()
        else
            tk.msg(refusetitle, refusetext)
            abort()
        end
    end
    player.unboard()
end

function accept()
    tk.msg(accepttitle, accepttext)

    misn.accept()
    accepted = true
    
    misn.setDesc(misn_desc)
    misn.setReward(misn_reward)
    
    osd_msg[1] = string.format(osd_msg[1], misssys[1]:name())
    misn.osdCreate(osd_title, osd_msg)
    
    hook.jumpin("jumpin")
    -- TODO
end

function jumpin()
    if accepted then
        if system.cur() == misssys[1] then
            -- Handle actual mission here
        elseif system.cur() == misssys[2] then
            -- Handle actual mission here
        elseif system.cur() == misssys[3] then
            -- Handle actual mission here
        elseif system.cur() == seirsys then
            seiryuu = pilot.add("Seiryuu", "trader", vec2.new(0, -2000), false)[1]
            seiryuu:setInvincible(true)
            if missend then
                seiryuu:disable()
                hook.pilot(seiryuu, "board", "board")
            end
        end
    end
end

function board()
    seiryuu:setHealth(100,100)
    -- TODO: end mission text, reward
    var.pop("shadowvigil_active")
end

function abort()
    var.pop("shadowvigil_active")
    misn.finish(false)
end