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

    NPCtitle = "No Jorek"
    NPCtext = [[You step into the bar, expecting to find Jorek McArthy sitting somewhere at a table. However, you don't see him anywhere. You decide to go for a drink to contemplate your next move. Then, you notice the barman is giving you a curious look.]]
    NPCdesc = "The barman seems to be eyeing you in particular."
    
    Jordesc = "There he is, Jorek McArthy, the man you've been chasing across half the galaxy. What he's doing on this piece of junk is unclear."
        
    -- Mission info stuff
    osd_title = {}
    osd_msg   = {}
    osd_title = "Dark Shadow"
    osd_msg[0] = "Look for Jorek on %s in the %s system" -- Note: indexing at 0 because it's a template.
    
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
    
    if not misn.claim ( {seirsys, ambushsys} ) then
        abort()
    end
    
    tk.msg(title[1], text[1]:format(player.name(), seirplanet:name(), seirsys:name()))
    accept() -- The player automatically accepts this mission.
end

-- This is the initial phase of the mission, when it still only shows up in the mission list. No OSD, reward or markers yet.
function accept()
    misn.setDesc(misn_desc1:format(seirsys:name(), seirplanet:name()))
    misn.accept()
    misn.osdDestroy() -- This is here because setDesc initializes the OSD.

    stage = 1

    hook.jumpin("jumpin") 
end

-- This is the "real" start of the mission. Get yer mission variables here!
function accept2()
    osd_msg[1] = osd_msg[0]:format(jorekplanet1:name(), joreksys1:name())
    misn.osdCreate(osd_title, osd_msg) 
    misn.setDesc(misn_desc2)
    misn.setReward(misn_reward)
    marker = misn.markerAdd(joreksys1, "low")
    landhook = hook.land("land", "bar")
end

-- Handle boarding of the Seiryuu
function board()
    if stage == 1 then -- Briefing
        tk.msg(title[2], text[2]:format(player.name()))
        tk.msg(title[2], text[3]:format(player.name()))
        tk.msg(title[2], text[4])
        tk.msg(title[2], text[5]:format(player.name(), jorekplanet1:name(), joreksys1:name(), jorekplanet1:name()))
        player.unboard()
        seiryuu:setHilight(false)
        accept2()
        stage = 2
    elseif stage == 5 then -- Debriefing
        player.unboard()
        seiryuu:setHilight(false)
        var.pop("darkshadow_active") 
        misn.finish(true)
    end
end

-- Jumpin hook
function jumpin()
    if system.cur() == seirsys then
        seiryuu = pilot.add("Seiryuu", nil, vec2.new(300, 300) + seirplanet:pos())[1]
        seiryuu:setInvincible(true)
        seiryuu:disable()
        if stage == 1 or stage == 5 then
            seiryuu:setHilight(true)
            hook.pilot(seiryuu, "board", "board")
        else
            seiryuu:setNoboard(true)
        end
    elseif system.cur() == joreksys2 and stage == 3 then
        -- TODO: Make some four winds ships lurk about
    elseif system.cur() == joreksys2 and stage == 4 then
        -- TODO: Cutscene where you are shown which ships to avoid
    elseif system.cur() == ambushsys and stage == 4 then
        -- TODO: Ambush by Genbu
    end
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
    tk.msg("lele", "Hi, I'm SHITMAN.")
    misn.npcRm(joreknpc)
    stage = 4
    hook.takeoff("takeoff")
end

-- Takeoff hook
function takeoff()
    -- SHITMAN found, inititate blockade run sequence.
end

-- Handle the unsuccessful end of the mission.
function abort()
    var.pop("darkshadow_active") 
    misn.finish(false)
end
