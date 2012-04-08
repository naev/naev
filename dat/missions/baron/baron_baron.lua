--[[
-- This is the first mission in the crazy baron string.
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    sysname1 = "Darkstone"
    sysname2 = "Ingot"
    planetname = "Varia"
    
    title = {}
    text = {}
    osd_msg = {}
    
    title[1] = "You've been scouted!"
    text[1] = [[    Your viewscreen flashes to life. You're greeted by a nondescript pilot who doesn't seem to be affiliated with anyone you know.
    "Hello there! I represent a man by the name of Baron Sauterfeldt. You may have heard of him in your travels? No? Well, I suppose you can't have it all. My employer is a moderately influential man, you see, and... But no, I'll not bore you with the details. The bottom line is, Lord Sauterfeldt is looking for hired help, and you seem like the sort he needs, judging by your ship."
    You inquire what it is exactly this Mr. Sauterfeldt needs from you.
    "Oh, nothing too terribly invasive, I assure you. His Lordship currently needs a courier, nothing more. Erm, well, a courier who can't be traced back to him, if you understand what I mean. So what do you think? Sounds like a suitable job for you? The pay is good, I can assure you that!"
    You pause for a moment before responding to this sudden offer. It's not everyday that people come to bring you work instead of making you look for it, but then again this job sounds like it could get you in trouble with the authorities. What will you do?]]

    title[2] = "A risky retrieval"
    text[2] = [[    "Oh, that's great! Okay, here's what Baron Sauterfeldt needs you to do. You should fly to the Dvaered world %s. There's an art museum dedicated to one of the greatest Warlords in recent Dvaered history. I forget his name. Drovan or something? Durvan? Uh, anyway. This museum has a holopainting of the Warlord and his military entourage. His Lordship really wants this piece of art, but the museum has refused to sell it to him. So, we've sent agents to... appropriate... the holopainting."
    You raise an eyebrow, but the pilot on the other end seems to be oblivious to the gesture.
    "So, right, you're going to %s to meet with our agents. You should find them in the spaceport bar. They'll get the item onto your ship, and you'll transport it out of Dvaered space. All quiet-like of course. No need for the authorities to know until you're long gone. Don't worry, our people are pros. It'll go off without a hitch, trust me."
    You smirk at that. You know from experience that things seldom 'go off without a hitch', and this particular plan doesn't seem to be all that well thought out. Still, it doesn't seem like you'll be in a lot of danger. If things go south, they'll go south well before you are even in the picture. And even if the authorities somehow get on your case, you'll only have to deal with the planetary police, not the entirety of House Dvaered.
    You ask the Baron's messenger where this holopainting needs to be delivered. "His Lordship will be taking your delivery in the %s system, aboard his ship the Pinnacle," he replies. "Once you arrive with the holopainting onboard your ship, hail the Pinnacle and ask for docking permission. They'll know who you are, so you should be allowed to dock. You'll be paid on delivery. Any questions?"
    You indicate that you know what to do, then cut the connection. Next stop: planet %s.]]

    title[3] = "Cloak and dagger"
    text[3] = [[    The three shifty-looking patrons regard you with apprehension as you approach their table. Clearly they don't know who their contact is supposed to be. You decide to be discreet, asking them if they've ever heard of a certain Sauterfeldt. Upon hearing this, the trio visibly relaxes. They tell you that indeed they know the man you speak of, and that they have something of his in their possession. Things proceed smoothly from that point, and several minutes later you are back at your ship, preparing it for takeoff while you wait for the agents to bring you your cargo.
    You're halfway through your pre-flight security checks when the three appear in your docking hangar. They have a cart with them on which sits a rectangular chest as tall as a man and as long as two. Clearly this holopainting is fairly sizeable. As you watch them from your bridge's viewport, you can't help but wonder how they managed to get something that big out of a Dvaered museum unnoticed.

    As it turns out, they didn't. They have only just reached the docking bridge leading into your ship when several armed Dvaered security forces come bursting into the docking hangar. They spot the three agents and immediately open fire. One of them goes down, the others hurriedly push the crate over the bridge towards your ship. Despite the drastic change in the situation, you have time to note that the Dvaered seem more interested in punishing the criminals than retrieving their possession intact.
    The second agent is caught by a Dvaered bullet, and topples off the docking bridge and into the abyss below. The third manages to get the cart with the chest into your airlock before catching a round with his chest as well. As the Dvaered near your ship, you seal the airlock, fire up your engines and punch it out of the docking hangar.]]

    title[4] = "Green light for docking"
    text[4] = [[Your comm is answered by a communications officer on the bridge of the Pinnacle. You tell her you've got a delivery for the baron. She runs a few checks on a console off the screen, then tells you you've been cleared for docking and that the Pinnacle will be brought to a halt.]]

    title[5] = "No bad deed goes unrewarded"
    text[5] = [[    When you arrive at your ship's airlock, the chest containing the Dvaered holopainting is already being carted onto the Pinnacle by a pair of crewmen. "You'll be wanting your reward, eh? Come along", one of them yells at you. They both chuckle and head off down the corridor..
    You follow the crewmen as they push the cart through the main corridor of the ship. Soon you arrive at a door leading to a large, luxurious compartment. You can tell at a glance that these are Baron Sauterfeldt's personal quarters. The Baron himself is present. He is a fat man, wearing a tailored suit that manages to make him look stately rather than pompous, a monocle and several rings on each finger. In a word, the Baron has a taste for the extravagant.
    "Ah, my holopainting," he coos as the chest is being carried into his quarters. "At last, I've been waiting forever." The Baron does not seem to be aware of your presence at all. He continues to fuss over the holopainting even as his crewman strip away the chest and lift the frame up to the wall.]]

    title[6] = "The Baron's Quarters"
    text[6] = [[    You look around his quarters. All sorts of exotic blades and other "art" works adorn his room, along with tapestries and various other holopaintings. You notice a bowl atop a velvet rug with "Fluffles" on it. Hanging above it seems to be a precariously balanced ancient blade.
    The crewmen finally unpack the holopainting. You glance at the three dimensional depiction of a Dvaered warlord, who seems to be discussing strategy with his staff. Unfortunately you don't seem to be able to appreciate Dvaered art, and you lose interest almost right away.
    You cough to get the Baron's attention. He looks up, clearly displeased at the disturbance, then notices you for the first time. "Ah, of course," he grunts. "I suppose you must be paid for your service. Here, have some credits. Now leave me alone. I have art to admire." The Baron tosses you a couple of credit chips, and then you are once again air to him. You are left with little choice but to return to your ship, undock, and be on your way.]]

    refusetitle = "Never the wiser"
    refusetext = [[    "Oh. Oh well, too bad. I'll just try to find someone who will take the job, then. Sorry for taking up your time. See you around!"]]

    choice1 = "Accept the job"
    choice2 = "Politely decline"

    comm1 = "All troops, engage %s %s! It has broken %s law!"

    -- Mission details
    misn_title = "Baron"
    misn_reward = "A tidy sum of money"
    misn_desc = "You've been hired as a courier for one Baron Sauterfeldt. Your job is to transport a holopainting from a Dvaered world to the Baron's ship."

    credits = 40000 -- 40K

    -- NPC stuff
    npc_desc = "These must be the 'agents' hired by this Baron Sauterfeldt. They look shifty. Why must people involved in underhanded business always look shifty?"

    -- OSD stuff
    osd_title = "Baron"
    osd_msg[1] = "Fly to the %s system and land on planet %s"
    osd_msg[2] = "Fly to the %s system and dock with (board) Kahan Pinnacle"
end

function create ()
    missys = {system.get("Darkstone")}
    if not misn.claim(missys) then
        abort()
    end

    if tk.choice(title[1], text[1], choice1, choice2) == 1 then
        accept()
    else
        tk.msg(refusetitle, refusetext)
        abort()
    end
end

function accept()
    tk.msg(title[2], text[2]:format(planetname, planetname, sysname2, planetname))
    
    misn.accept()
    
    misn.setTitle(misn_title)
    misn.setReward(misn_reward)
    misn.setDesc(misn_desc)
    
    osd_msg[1] = osd_msg[1]:format(sysname1, planetname)
    osd_msg[2] = osd_msg[2]:format(sysname2)
    misn.osdCreate(osd_title, osd_msg)
   
    misn_marker = misn.markerAdd( system.get(sysname1), "low" )
    
    talked = false
    stopping = false
    
    hook.land("land")
    hook.jumpin("jumpin")
    hook.takeoff("takeoff")
end

function land()
    if planet.cur() == planet.get(planetname) and not talked then
        thief1 = misn.npcAdd("talkthieves", "Sauterfeldt's agents", "neutral/thief1", npc_desc)
        thief2 = misn.npcAdd("talkthieves", "Sauterfeldt's agents", "neutral/thief2", npc_desc)
        thief3 = misn.npcAdd("talkthieves", "Sauterfeldt's agents", "neutral/thief3", npc_desc)
    end
end

function jumpin()
    if talked and system.cur() == system.get(sysname2) then
        pinnacle = pilot.add("Proteron Kahan", "trader", planet.get("Ulios"):pos() + vec2.new(-400,-400))[1]
        pinnacle:setFaction("Civilian")
        pinnacle:rename("Pinnacle")
        pinnacle:setInvincible(true)
        pinnacle:control()
        pinnacle:setHilight(true)
        pinnacle:goto(planet.get("Ulios"):pos() + vec2.new( 400, -400), false)
        idlehook = hook.pilot(pinnacle, "idle", "idle")
        hook.pilot(pinnacle, "hail", "hail")
    end
end

function idle()
    pinnacle:goto(planet.get("Ulios"):pos() + vec2.new( 400,  400), false)
    pinnacle:goto(planet.get("Ulios"):pos() + vec2.new(-400,  400), false)
    pinnacle:goto(planet.get("Ulios"):pos() + vec2.new(-400, -400), false)
    pinnacle:goto(planet.get("Ulios"):pos() + vec2.new( 400, -400), false)
end

function hail()
    if talked then
        tk.msg(title[4], text[4])
        pinnacle:taskClear()
        pinnacle:brake()
        pinnacle:setActiveBoard(true)
        hook.pilot(pinnacle, "board", "board")
        hook.rm(idlehook)
    end
end

function board()
    tk.msg(title[5], text[5])
    tk.msg(title[6], text[6])
    player.pay( credits )
    player.refuel()
    player.unboard()
    pinnacle:control(false)
    pinnacle:changeAI("flee")
    pinnacle:setHilight(false)
    pinnacle:setActiveBoard(false)
    misn.finish(true)
end

function talkthieves()
    tk.msg(title[3], text[3])

    misn.npcRm(thief1)
    misn.npcRm(thief2)
    misn.npcRm(thief3)

    talked = true
    carg_id = misn.cargoAdd("The Baron's holopainting", 0)

    misn.osdActive(2)
    misn.markerMove( misn_marker, system.get(sysname2) )

    player.takeoff()
end

function takeoff()
    if talked and system.cur() == system.get(sysname1) then
        vendetta1 = pilot.add("Dvaered Vendetta", "dvaered_norun", vec2.new(500,0))[1]
        vendetta2 = pilot.add("Dvaered Vendetta", "dvaered_norun", vec2.new(-500,0))[1]
        vendetta1:rename("Dvaered Police Vendetta")
        vendetta2:rename("Dvaered Police Vendetta")
        vendetta1:control()
        vendetta2:control()
        vendetta1:attack(player.pilot())
        vendetta2:attack(player.pilot())
        vendetta1:broadcast(comm1:format(player.pilot():ship():baseType(), player.ship(), planetname), true)
    end
end

function abort()
    misn.finish(false)
end
