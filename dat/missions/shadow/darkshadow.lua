--[[
-- This is the third mission in the "shadow" series, featuring the return of SHITMAN.
--]]

include ("scripts/proximity.lua")

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

    title = {}
    text = {}
    Jorscene = {}

    title[1] = "An urgent invitation"
    text[1] = [[    Suddenly, out of nowhere, one of the dormant panels in your cockpit springs to life. It shows you a face you've never seen before in your life, but you recognize the plain grey uniform as belonging to the Four Winds.
    "Hello %s," the face says. "You must be wondering who I am and how it is I'm talking to you like this. Neither question is important. What is important is that captain Rebina has urgent need of your services. You are to meet her on the Seiryuu, which is currently in orbit around %s in the % system. Please don't ask any questions now. We expect to see you as quickly as you can make your way here."
    The screen goes dead again. You decide to make a note of this in your log. Perhaps it would be a good idea to visit the Seiryuu once more, if only to find out how they got a private line to your ship!]]

    title[2] = "Disclosure"
    text[2] = [[    You make your way through the now familiar corridors of the Seiryuu. You barely notice the strange environment anymore. It seems unimportant compared to the strange events that surround your every encounter with these Four Winds.
    You step onto the bridge, where captain Rebina is waiting for you. "Welcome back, %s," she says. "I'm pleased to see that you decided to respond to our communication. I doubt you would have come here if you weren't willing to continue to aid us. Your presence here confirms that you are a reliable partner, so I will treat you accordingly."
    the captain motions you to take a seat at what looks like a holotable in the center of the bridge. "Before I tell you what I've called you here for, I feel I should explain to you in full who we are, what we do and what your part in all this is." She takes a seat opposite from yours, and leans on the holotable. "As I've said before, we are the Four Winds. Our organization is a very secretive one, as you've experienced firsthand. Very few outside our know ranks of our existence, and now you're one of those few."]]

    text[3] = [[    "The Four Winds are old, %s. Very old indeed. The movement dates back to old Earth, before the Space Age, even. We have been with human civilization throughout the ages, at first only in the Eastern nations, later establishing a foothold worldwide. Our purpose was to guide humanity, prevent it from making mistakes it could not afford to make. We never came out in the open, we always worked behind the scenes, from the shadows. We were diplomats, scientists, journalists, politicians' spouses, sometimes even assassins. We used any means necessary to gather information and avert disaster, when we could.
    "Of course, we didn't always succeed. We couldn't prevent the nuclear strikes on Japan, though we managed to prevent several others. We foiled the sabotage attempts on several of the colony ships launched during the First Growth, but sadly failed to do so in Wendigo's case. We failed to stop the Faction Wars, though we managed to help the Empire gain the upper hand. Our most recent failure is the Incident - we should have seen it coming, but we were completely taken by surprise."]]

    text[4] = [[    Captain Rebina sits back in her chair and heaves a sigh. "I think that may have been when things started to change. We used to be committed to our purpose, but apparently things are different now. No doubt you remember what happened to the diplomatic exchange between the Empire and the Dvaered some time ago. Well, suffice to say that increasing the tension between the two is definitely not part of our mandate. In fact, it's completely at odds with what we stand for. And that was not just an isolated incident either. Things have been happening that suggest Four Winds involvement, things that bode ill."
    She activates the holotable, and it displays four cruisers, all seemingly identical to the Seiryuu, though you notice subtle differences in the hull designs.
    "These are our flagships. Including this ship, they are the Seiryuu, Suzaku, Byakko and Genbu. I'm given to understand that these names, as well as our collective name, have their roots in ancient Oriental mythology." The captain touches another control, and four portraits appear, superimposed over the ships. "These are the four captains of the flagships, which by extension makes them the highest level of authority within the Four Winds. You know me. The other three are called Giornio, Zurike and Farett."]]

    text[5] = [[    "It is my belief that one or more of my fellow captains have abandoned their mission, and are misusing their resources for a different agenda. I have been unable to find out the details of Four Winds missions that I did not order myself, which is a bad sign. I am being stonewalled, and I don't like it. I want to know what's going on, %s, and you're going to help me do it."
    The captain turns the holotable back off so she can have your undivided attention. "I have sent Jorek on a recon mission to the planet of %s in the %s system. He hasn't reported back to me so far, and that's bad news. Jorek is a reliable agent. If he fails to meet a deadline, then it means he is tied down by factors outside of his control, or worse. I want you to find him. Your position as an outsider will help you fly below the radar of potentially hostile Four Winds operatives. You must go to %s and contact Jorek if you can, or find out where he is if you can't."
    Captain Rebina stands up, a signal that this briefing is over. You are seen to your ship by a grey-uniformed crewman. You sit in your cockpit for a few minutes before disengaging the cocking clamp. What captain Rebina has told you is a lot to take in. A shadowy organization that guides humanity behind the scenes? And parts of that organization going rogue? The road ahead could well be a bumpy one.]]

    title[3] = "A tip from the barman"
    text[6] = [[    You meet the barman's stare. He hesitates for a moment, then speaks up.
    "Hey man. Are you %s by any chance?"
    You tell him that yes, that's you, and ask how he knows your name.
    "Well, your description was given to me by an old friend of mine. His name is Jarek. Do you know him?"
    You tell him that you don't know anyone by the name of Jarek, but you do know a man named Jorek. The barman visibly relaxes when he hears that name.
    "Ah, good. You're the real deal then. Can't be too careful in times like these, you know. Anyway, old Jorek was here, but he couldn't stay. He told me to keep an eye out for you, said you'd be coming to look for him." The barman glances around to make sure nobody is within earshot, even though the bar's music makes it difficult to overhear anyone who isn't standing right next to you. "I have a message for you. Go to the %s system and land on %s. Jorek will be waiting for you there. But you better be ready for some trouble. I don't know what kind of trouble it is, but Jorek is never in any kind of minor trouble. Don't say I didn't warn you."
    You thank the barman, pay for your drink and prepare to head back to your ship, wondering whether your armaments will be enough to deal with whatever trouble Jorek is in.]]

    title[4] = "Still an unpleasant man"
    text[7] = [[    "Well hello there %s," Jorek says when you approach his table. "It's about damn time you showed up. I've been wastin' credits on this awful swill for days now."
    Not at all surprised that Jorek is still as disagreeable as the last time you encountered him, you decide to ask him to explain the situation, beginning with how he knew that it was you who would be coming for him. Jorek laughs heartiily at that.
    "Ha! Of course it was going to be you. Who else would that lass Rebina send? She's tough as nails, that girl, but I know how her mind works. She's cornered, potential enemies behind every door in the organization. And you have done us a couple of favors already. In fact, you're the only guy she can trust outside her own little circle of friends, and right now I'm not too sure how far she trusts those. Plus, she really has a keen nose when it comes to sniffin' out reliable people, and she knows it. Yeah, I knew she'd send you to find me."
    That answers one question. But you still don't know why Jorek hasn't been reporting in like he should have.
    "Yeah, right, about that. You know about the deal with the other branches getting too big for their britches? Good. Well, I've been lookin' into that, pokin' my nose into their business. Since I'm dealin' with my fellow Shadows here, I couldn't afford to give myself away. So that's that. But there's more."]]

    text[8] = [[    I dunno if you've seen them on your way here, but there's guys of ours hangin' around in the system. And when I say guys of ours, I mean guys of theirs, since they sure ain't our guys any more. They've been on my ass ever since I left Manis, so I think I know what they want. They want to get me and see what I know, or maybe they just want to blow me into space dust. Either way, I need you to help me get out of this rothole."
    You ask Jorek why he didn't just lie low on some world until the coast was clear, instead of coming to this sink for the dregs of intergalactic society.
    "It ain't that simple," Jorek sighs. "See, I got an inside man. A guy in their ranks who wants out. I need to get him back to the old girl so he can tell her what he knows firsthand. He's out there now, with the pack, so we need to pick him up on our way out. Now, there's two ways we can do this. We can either go in fast, grab the guy, get out fast before the wolves get us. Or we can try to fight our way through. Let me warn you though, these guys mean business, and they're not your average pirates. Unless you got a really tough ship, I recommend you run."
    Jorek sits back in his chair. "Well, there you have it. I'll fill you in on the details once we're spaceborne. Show me to your ship, buddy, and let's get rollin' I've had enough of this damn place."]]

    title[5] = "An extra passenger"
    text[9] = [[    You board with the Four Winds vessel, and as soon as the airlock opens a nervous looking man enters your ship. He eyes you warily, but when he sees that Jorek is with you his tension fades.
    "Come on, %s," Jorek says. "Let's not waste any more time here. We got what we came for. Now let's give these damn vultures the slip, eh?"]]
    
    title[6] = "Ambush!"
    text[10] = [[   Suddenly, your long range sensors pick up a ship jumping in behind you. Jorek checks the telemetry beside you. Suddenly, his eyes go wide and he groans. The Four Winds informant turns pale.
    "Oh, damn it all," Jorek curses. "%s, that's the Genbu, Giornio's flagship. I never expected him to take an interest in me personally! Damn, this is bad. Listen, if you have anything to boost our speed, now would be the time. We got to get outta here as if all hell was hot on our heels, which it kinda is! If that thing catches us, we're toast. I really mean it, you don't wanna get into a fight against her, not on your own. Get your ass movin' to Sirius space. Giornio ain't gonna risk getting into a scrap with the Sirius military, so we'll be safe once we get there. Come on, what are you waitin' for? Step on it!"]]
    
    title[7] = "A safe return"
    text[11] = [[   You find yourself back on the Seiryuu, in the company of Jorek and the Four Winds informant. The informant is escorted deeper into the ship by grey-uniformed crew members, while Jorek takes you up to the bridge for a meeting with captain Rebina.
    "Welcome back, Jorek, %s," Rebina greets you on your arrival. "I've already got a preliminary report on the situation, but let's have ourselves a proper debriefing. Have a seat."
    Jorek and you sit down at the holotable in the middle of the bridge, and report on the events surrounding Jorek's retrieval. When you're done, captain Rebina calls up a schematic view of the Genbu from the holotable.
    "It would seem that Giornio and his comrades have a vested interest in keeping me away from the truth. It's a good thing you managed to get out of that ambush and bring me that informant. I do hope he'll be able to shed more light on the situation. I've got a bad premonition, a hunch that we're going to have to act soon if we're going to avert disaster, whatever that may be. I trust that you will be willing to aid us again when that time comes, %s. We're going to need all the help we can get. For now, you will find a modest amount of credits in your account. I will be in touch when things are clearer."
    You return to your ship and undock from the Seiryuu. You reflect that you had to run for your life this time around, and by all accounts, things will only get worse with the Four Winds in the future. A lesser man might get nervous.]]

    Jorscene[1] = [[Jorek> "That's my guy. We got to board his ship and get him off before we jump."]]
    Jorscene[2] = [[Jorek> "Watch out for those patrols though. If they spot us, they'll be all over us."]]
    Jorscene[3] = [[Jorek> "They're tougher than they look. Don't underestimate them."]]

    joefailtitle = "You forgot the informant!"
    joefailtext = [[Jorek is enraged. "Dammit, %s! I told you to pick up that informant on the way! Too late to go back now. I'll have to think of somethin' else. I'm disembarkin' at the next spaceport, don't bother taking me back to the Seiryuu."]]

    patrolcomm = "All pilots, we've detected McArthy on that ship! Break and intercept!"
    
    NPCtitle = "No Jorek"
    NPCtext = [[You step into the bar, expecting to find Jorek McArthy sitting somewhere at a table. However, you don't see him anywhere. You decide to go for a drink to contemplate your next move. Then, you notice the barman is giving you a curious look.]]
    NPCdesc = "The barman seems to be eyeing you in particular."

    Jordesc = "There he is, Jorek McArthy, the man you've been chasing across half the galaxy. What he's doing on this piece of junk is unclear."

    -- Mission info stuff
    osd_title = {}
    osd_msg   = {}
    osd2_msg  = {}
    osd_title = "Dark Shadow"
    osd_msg[0] = "Look for Jorek on %s in the %s system" -- Note: indexing at 0 because it's a template. Shouldn't actually appear ingame.
    osd2_msg[1] = "Fetch the Four Winds informant from his ship"
    osd2_msg[2] = "Return Jorek and the informant to the Seiryuu in the %s system"

    misn_desc1 = [[You have been summoned to the %s system, where the Seiryuu is supposedly waiting for you in orbit around %s.]]
    misn_desc2 = [[You have been tasked by captain Rebina of the Four Winds to assist Jorek McArthy.]]
    misn_reward = "A sum of money."
end

function create()
    var.push("darkshadow_active", true)

    seirplanet, seirsys = planet.get("Edergast")
    jorekplanet1, joreksys1 = planet.get("Manis")
    jorekplanet2, joreksys2 = planet.get("The Wringer")
    ambushsys = system.get("Herakin")
    safesys = system.get("Eiderdown")

    if not misn.claim ( {seirsys, joreksys2, ambushsys} ) then
        abort()
    end

    tk.msg(title[1], text[1]:format(player.name(), seirplanet:name(), seirsys:name()))
    firstmarker = misn.markerAdd(seirsys, "low")
    accept() -- The player automatically accepts this mission.
end

-- This is the initial phase of the mission, when it still only shows up in the mission list. No OSD, reward or markers yet.
function accept()
    misn.setDesc(misn_desc1:format(seirsys:name(), seirplanet:name()))
    misn.accept()
    misn.osdDestroy() -- This is here because setDesc initializes the OSD.

    stage = 1

    hook.enter("enter")
end

-- This is the "real" start of the mission. Get yer mission variables here!
function accept2()
    tick = {false, false, false, false, false}
    tick["__save"] = true
    osd_msg[1] = osd_msg[0]:format(jorekplanet1:name(), joreksys1:name())
    misn.osdCreate(osd_title, osd_msg)
    misn.setDesc(misn_desc2)
    misn.setReward(misn_reward)
    marker = misn.markerAdd(joreksys1, "low")
    landhook = hook.land("land", "bar")
    jumpouthook = hook.jumpout("jumpout")
end

-- Handle boarding of the Seiryuu
function seiryuuBoard()
    seiryuu:setActiveBoard(false)
    seiryuu:setHilight(false)
    player.unboard()
    if stage == 1 then -- Briefing
        tk.msg(title[2], text[2]:format(player.name()))
        tk.msg(title[2], text[3]:format(player.name()))
        tk.msg(title[2], text[4])
        tk.msg(title[2], text[5]:format(player.name(), jorekplanet1:name(), joreksys1:name(), jorekplanet1:name()))
        accept2()
        misn.markerRm(firstmarker)
        stage = 2
    elseif stage == 6 then -- Debriefing
        tk.msg(title[7], text[11]:format(player.name(), player.name()))
        player.pay(500000) -- 500K
        seiryuu:control()
        seiryuu:hyperspace()
        var.pop("darkshadow_active")
        misn.finish(true)
    end
end

-- Board hook for joe
function joeBoard()
    tk.msg(title[5], text[9]:format(player.name()))
    misn.cargoAdd("Four Winds Informant", 0)
    player.unboard()
    misn.markerMove(marker, seirsys)
    misn.osdActive(2)
    stage = 5
end

-- Jumpout hook
function jumpout()
    playerlastsys = system.cur() -- Keep track of which system the player came from
    if not (patroller == nil) then
        hook.rm(patroller)
    end
    if not (spinter == nil) then
        hook.rm(spinter)
    end
end

-- Enter hook
function enter()
    if system.cur() == seirsys then
        seiryuu = pilot.add("Seiryuu", nil, vec2.new(300, 300) + seirplanet:pos())[1]
        seiryuu:setInvincible(true)
        seiryuu:control()
        if stage == 1 or stage == 6 then
            seiryuu:setActiveBoard(true)
            seiryuu:setHilight(true)
            hook.pilot(seiryuu, "board", "seiryuuBoard")
        else
            seiryuu:setNoboard(true)
        end
    elseif system.cur() == joreksys2 and stage == 3 then
        pilot.clear()
        pilot.toggleSpawn(false)
        spawnSquads(false)
    elseif system.cur() == joreksys2 and stage == 4 then
        pilot.clear()
        pilot.toggleSpawn(false)
        player.allowLand(false, "Landing permission denied. Our docking clamps are currently undergoing maintenance.")
        -- Meet Joe, our informant.
        joe = pilot.add("Four Winds Vendetta", nil, vec2.new(-500, -4000))[1]
        joe:control()
        joe:rename("Four Winds Informant")
        joe:setHilight(true)
        joe:setVisplayer()
        joe:setInvincible(true)
        joe:disable()
        spawnSquads(true)

        -- Make everyone visible for the cutscene
        squadVis(true)

        -- The cutscene itself
        local delay = 0
        zoomspeed = 2500
        hook.timer(delay, "playerControl", true)
        delay = delay + 2000
        hook.timer(delay, "zoomTo", joe)
        delay = delay + 4000
        hook.timer(delay, "showText", Jorscene[1])
        delay = delay + 4000
        hook.timer(delay, "zoomTo", leader[1])
        delay = delay + 1000
        hook.timer(delay, "showText", Jorscene[2])
        delay = delay + 2000
        hook.timer(delay, "zoomTo", leader[2])
        delay = delay + 3000
        hook.timer(delay, "zoomTo", leader[3])
        delay = delay + 2000
        hook.timer(delay, "showText", Jorscene[3])
        delay = delay + 3000
        hook.timer(delay, "zoomTo", leader[4])
        delay = delay + 4000
        hook.timer(delay, "zoomTo", leader[5])
        delay = delay + 4000
        hook.timer(delay, "zoomTo", player.pilot())
        hook.timer(delay, "playerControl", false)

        -- Hide everyone again
        delay = delay + 2000
        hook.timer(delay, "squadVis", false)
        delay = delay + 1
        -- ...except the leadears.
        hook.timer(delay, "leaderVis", true)

        hook.pilot(joe, "board", "joeBoard")
        poller = hook.timer(500, "patrolPoll")
    elseif system.cur() == ambushsys and stage == 4 then
        tk.msg(joefailtitle, joefailtext:format(player.name()))
        abort()
    elseif system.cur() == ambushsys and stage == 5 then
        pilot.clear()
        pilot.toggleSpawn(false)
        hook.timer(500, "invProximity", { location = system.cur():jumpPos(system.get("Suna")), radius = 8000, funcname = "startAmbush" }) -- Starts an inverse proximity poll for distance from the jump point.
    elseif system.cur() == safesys and stage == 5 then
        stage = 6 -- stop spawning the Genbu
    elseif genbuspawned and stage == 5 then
        spawnGenbu(playerlastsys) -- The Genbu follows you around, and will probably insta-kill you.
        continueAmbush()
    end
end

function spawnSquads(highlight)
    -- Start positions for the leaders
    leaderstart = {}
    leaderstart[1] = vec2.new(-2500, -1500)
    leaderstart[2] = vec2.new(2500, 1000)
    leaderstart[3] = vec2.new(-3500, -4500)
    leaderstart[4] = vec2.new(2500, -2500)
    leaderstart[5] = vec2.new(-2500, -6500)

    -- Leaders will patrol between their start position and this one
    leaderdest = {}
    leaderdest[1] = vec2.new(2500, -1000)
    leaderdest[2] = vec2.new(-500, 1500)
    leaderdest[3] = vec2.new(-4500, -1500)
    leaderdest[4] = vec2.new(2000, -6000)
    leaderdest[5] = vec2.new(1000, -1500)

    squads = {}
    squads[1] = pilot.add("Four Winds Vendetta Quad", nil, leaderstart[1])
    squads[2] = pilot.add("Four Winds Vendetta Quad", nil, leaderstart[2])
    squads[3] = pilot.add("Four Winds Vendetta Quad", nil, leaderstart[3])
    squads[4] = pilot.add("Four Winds Vendetta Quad", nil, leaderstart[4])
    squads[5] = pilot.add("Four Winds Vendetta Quad", nil, leaderstart[5])

    for _, squad in ipairs(squads) do
        for _, k in ipairs(squad) do
            hook.pilot(k, "attacked", "attacked")
            k:rename("Four Winds Patrol")
            k:control()
            k:rmOutfit("all")
            k:addOutfit("Cheater's Laser Cannon", 6) -- Equip these fellas with unfair weaponry
            k:follow(squad[1]) -- Each ship in the squad follows the squad leader
        end
        squad[1]:taskClear() --...except the leader himself.
    end
    
    -- Shorthand notation for the leader pilots
    leader = {}
    leader[1] = squads[1][1]
    leader[2] = squads[2][1]
    leader[3] = squads[3][1]
    leader[4] = squads[4][1]
    leader[5] = squads[5][1]

    leaderVis(highlight)

    -- Kickstart the patrol sequence
    for i, j in ipairs(leader) do
        j:goto(leaderdest[i], false)
    end

    -- Set up the rest of the patrol sequence
    for _, j in ipairs(leader) do
        hook.pilot(j, "idle", "leaderIdle")
    end
end

-- Makes the squads either visible or hides them
function squadVis(visible)
    for _, squad in ipairs(squads) do
        for _, k in ipairs(squad) do
            k:setVisplayer(visible)
        end
    end
end

-- Makes the leaders visible or hides them, also highlights them (or not)
function leaderVis(visible)
    for _, j in ipairs(leader) do
        j:setVisplayer(visible)
        j:setHilight(visible)
    end
end

-- Hook for hostile actions against a squad member
function attacked()
    for _, squad in ipairs(squads) do
        for _, k in ipairs(squad) do
            k:hookClear()
            k:control()
            k:attack(player.pilot())
        end
    end
end

-- Hook for the idle status of the leader of a squad.
-- Makes the squads patrol their routes.
-- TODO: make this shorter
function leaderIdle(pilot)
    for i, j in ipairs(leader) do
        if j == pilot then
            if tick[i] then pilot:goto(leaderdest[i], false)
            else pilot:goto(leaderstart[i], false)
            end
            tick[i] = not tick[i]
            return
        end
    end
end

-- Check if any of the patrolling leaders can see the player, and if so intercept.
function patrolPoll()
    for _, patroller in ipairs(leader) do
        if vec2.dist(player.pilot():pos(), patroller:pos()) < 1200 then
            patroller:broadcast(patrolcomm)
            attacked()
            return
        end
    end
    poller = hook.timer(500, "patrolPoll")
end

-- Spawns the Genbu
function spawnGenbu(sys)
    genbu = pilot.add("Genbu", nil, sys)[1]
    genbu:rmOutfit("all")
    genbu:addOutfit("Turbolaser", 3)
    genbu:addOutfit("Cheater's Ragnarok Beam", 3) -- You can't win. Seriously.
    genbu:control()
    genbu:setHilight()
    genbu:setVisplayer()
    genbuspawned = true
end

-- The initial ambush cutscene
function startAmbush()
    spawnGenbu(system.get("Anrique"))
    
    local delay = 0
    zoomspeed = 4500
    hook.timer(delay, "playerControl", true)
    hook.timer(delay, "zoomTo", genbu)
    delay = delay + 5000
    hook.timer(delay, "showMsg", {title[6], text[10]:format(player.name())})
    delay = delay + 1000
    hook.timer(delay, "zoomTo", player.pilot())
    hook.timer(delay, "playerControl", false)
    hook.timer(delay, "continueAmbush")
end

-- The continuation of the ambush, for timer purposes
function continueAmbush()
    genbu:setHostile()
    genbu:attack(player.pilot())
    -- TODO: launch interceptors
    spinter = hook.timer(5000, "spawnInterceptors")
end

-- Spawns a wing of Vendettas that intercept the player.
function spawnInterceptors()
    inters = pilot.add("Four Winds Lancelot Trio", nil, genbu:pos())
    for _, j in ipairs(inters) do
        j:rmOutfit("all")
        j:addOutfit("Cheater's Laser Cannon", 4) -- Equip these fellas with unfair weaponry
        j:addOutfit("Engine Reroute", 1)
        j:addOutfit("Improved Stabilizer", 1)
        j:control()
        j:attack(player.pilot())
    end
    spinter = hook.timer(30000, "spawnInterceptors")
end

-- Land hook
function land()
    if planet.cur() == jorekplanet1 and stage == 2 then
        -- Thank you player, but our SHITMAN is in another castle.
        tk.msg(NPCtitle, NPCtext)
        barmanNPC = misn.npcAdd("barman", "Barman", "thief2", NPCdesc, 4)
    elseif planet.cur() == jorekplanet2 and stage == 3 then
        joreknpc = misn.npcAdd("jorek", "Jorek", "jorek", Jordesc, 4)
    end
end

-- NPC hook
function barman()
    tk.msg(title[3], text[6]:format(player.name(), joreksys2:name(), jorekplanet2:name()))
    osd_msg[1] = osd_msg[0]:format(jorekplanet2:name(), joreksys2:name())
    misn.osdCreate(osd_title, osd_msg)
    misn.markerMove(marker, joreksys2)
    misn.npcRm(barmanNPC)
    stage = 3
end

-- NPC hook
function jorek()
    tk.msg(title[4], text[7]:format(player.name()))
    tk.msg(title[4], text[8])
    misn.npcRm(joreknpc)
    misn.cargoAdd("Jorek", 0)

    osd2_msg[2] = osd2_msg[2]:format(seirsys:name())
    misn.osdCreate(osd_title, osd2_msg)

    stage = 4
end

-- Capsule function for camera.set, for timer use
function zoomTo(target)
    camera.set(target, true, zoomspeed)
end

-- Capsule function for player.msg, for timer use
function showText(text)
    player.msg(text)
end

-- Capsule function for tk.msg, for timer use
function showMsg(content)
    tk.msg(content[1], content[2])
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

-- Poll for player proximity to a point in space. Will trigger when the player is NOT within the specified distance.
-- argument trigger: a table containing:
-- location: The target location
-- radius: The radius around the location
-- funcname: The name of the function to be called when the player is out of proximity.
function invProximity(trigger)
    if vec2.dist(player.pos(), trigger.location) >= trigger.radius then
        _G[trigger.funcname]()
    else
        hook.timer(500, "invProximity", trigger)
    end
end

-- Handle the unsuccessful end of the mission.
function abort()
    var.pop("darkshadow_active")
    misn.finish(false)
end
