--[[
-- This is the main script for the Shadowrun mission. It's started from the spaceport bar and tries to emulate spaceport bar conversation as part of the mission.
-- "shadowrun" stack variable:
-- 1 = player has met Rebina, but hasn't accepted the mission
-- 2 = player has accepted Rebina's mission, but has not talked to SHITMAN
-- 3 = player has talked to SHITMAN
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

   planetname = "Durea" -- The planet where SHITMAN lives
   planet = planet.get(planetname)
   sysname = "Capricorn" -- The system the planet is part of
   sys = system.get(sysname)
   sysname2 = "Uhriabi" -- The system where the ship is
   sys2 = system.get(sysname2)
   shipname = "Seiryuu"
   
   title = {}
   text = {}
   
   -- Pre-mission
   title[1] = "A dark-haired woman"
   title[2] = "Rebina's proposition"
   title[3] = "Rebina's explanation"
   text[1] = [[The woman calmly watches you as you approach her, seemingly not at all surprised to see you. Clad in a plain yet expensive-looking black dress and sipping from her martini, she emits an aura of class that is almost intimidating.
   "Hello," she greets you. "I had a feeling you might want to talk to me. You are different from most..." she gestures at the other patrons of the bar, "And so am I. But where are my manners, I haven't introduced myself. My name is Rebina. I am what you might call a talent hunter. I visit places such as these to find people of exceptional talent. People such as you."
   You begin to introduce yourself, but Rebina waves it away, perhaps because your name doesn't interest her, or possibly because she already knows who you are. "Let's not waste words on idle formalities," she says. "I am here to talk business, and I've got a proposition for you, if you're interested."]]
   text[2] = [[Rebina nods at you to acknowledge your existence. "We meet again. I'm glad to see you've not gotten yourself killed yet." She smiles meaningfully. "As it happens I haven't found anyone to take care of my business yet. Perhaps you would reconsider? Allow me to remind you what this is about."]]
   text[3] = [["What I need is a pilot and a ship. Specifically, I need a skilled pilot and a capable ship. Do you fit that description? I have a feeling you do. You see, what I am about to suggest you do is both profitable and dangerous." Rebina takes another sip of her drink before continuing, allowing what she just said to fully register. "I will not lie to you. There are... rivalries out there, and working for me will mean you'll take side in some of them. People will take notice of you, and some of them will try to kill you."
   You explain that taking risks comes with being an independent pilot and that you took the captain's chair with appropriate resolve, but Rebina pins you with a piercing gaze. "These are no ordinary pirate raids we're talking about," she admonishes you. "If you take this assignment, you will be a painted target. I want you to be well aware of this." There is another pause, but then she continues in a milder tone of voice. "That being said, I can assure you that the reward is well worth the risk. Pull this off, and you'll walk away considerably richer than you were."
   Rebina leans back, levelly meeting your gaze. "That's all I can tell you at this point. You'll get more details only once you accept this job. If you accept this job. What say you?"]]
   text[4] = [["Wonderful!" Rebina gives you a warm, sincere smile. "I don't mind admitting that it isn't easy finding pilots who measure up to my expectations, and finding ones willing to take a risk is more difficult still. I am pleased indeed."
   Then Rebina's expression changes to that of a businesswoman about to ply her trade. "Now, listen up. Contrary to what you may have thought, this assignment isn't about me. It's about a man who goes by the name of Jorek McArthy. The current state of affairs is that Jorek is staying on %s in the %s system, and this is not where me and my associates want him to be. Unfortunately, Jorek has attracted some unwanted attention, and we don't want him to focus that attention to us."
   Rebina takes a moment to sip from her drink. "I think you can see where this is going. You are to rendezvous with Jorek, take him aboard your ship, lose whoever's tailing him, then bring him to the %s system. There you will dock with one of our ships, the %s, which will take Jorek to his final destination. You will receive your reward from her captain once Jorek is aboard."
   "It's a simple objective, but accomplishing it might require considerable skill." She leans back and smiles. "Still, I have utmost confidence that you can do it. I seldom misjudge those I choose to trust."]]
   text[5] = [["You know what to do," Rebina tells you. "You will find Jorek in the spaceport bar on %s. When you see him, tell him you've come to 'see to his special needs'. Oh, and please be discrete. Don't talk about things you don't need to; the walls have ears in that place. In particular, don't mention any names."
   "You will be on a time schedule. You must meet Jorek within %d STU, or he will assume you are not coming and go back into hiding. You must also be at the meeting point %d STU from now. If you fail to meet with Jorek within the time limit or if you are prevented from taking him offworld for any other reason, make your way to the %s and report what happened. We'll take it from there. If you fail to show up at the designated time, we will assume you have failed, and the %s will leave."
   Rebina empies her glass and places it on the bar before rising to her feet. "That will be all. Good luck, and keep your wits about you."
   Then Rebina takes her leave from you and gracefully departs the spaceport bar. You order yourself another drink. You've got the feeling you're going to need it.]]
   refusal = [["I see. What a shame." Rebina's demeanor conveys that she's disappointed but not upset. "I can understand your decision. One should not bite off more than one can chew, after all. It seems I will have to try to find another candidate." She tilts her head slightly. Then, "Although if you change your mind before I do, you're welcome to seek me out again. I'll be around."
   Rebina finishes her drink and gets up. Then, with a cordial wave of her hand she sweeps out of the door. You momentarily regret not taking her up on her offer, but it passes. You've made the right decision, and that is that.]]

   -- Post-mission
   title[4] = "Empty-handed"
   title[5] = "An unexpected reunion"
   text[6] = [[You complete docking operations with the %s, well aware that your ship isn't carrying the man they were expecting. At the airlock, you are greeted by a pair of crewmen in grey uniforms. You explain to them that you were unable to bring Jorek to them, and they receive your report in a dry, businesslike manner. The meeting is short. The crewmen disappear back into their ship, closing the airlock behind them, and you return to your bridge.
   You prepare to undock from the %s, but before you complete the procedures there is a sudden power spike in your primary systems. All panels go black. In the darkness, the only thing that disturbs the silence is the sound of the %s dislodging itself from your docking clamp.
   Seconds later, the computer core reboots itself and your controls come back online, but you find to your dismay that your OS has been reset to factory defaults. All custom content has been lost - including your logs of meeting the %s...]]
   text[7] = [[You complete docking operations with the %s, well aware that your ship isn't carrying the man they were expecting. When the airlock opens, you find yourself face to face with a woman and two crewmen, all wearing grey, featureless uniforms. It takes you a few moments to realize that the woman is in fact Rebina. But this is not the elegant, feminine figure you met in the spaceport bar not too long ago. This woman emits an aura of authority, and you immediately understand that Rebina is in fact captain of the %s.
   "Well met, %s," she greets you. At the same time, the two crewmen that accompanied her push their way past you and disappear in the direction of your cargo hold. You open your mouth to protest, but Rebina raises a hand to forestall you. "There is no cause for concern," she says. "My men are only retrieving that which we sent you to fetch. I assure you that your ship and its cargo will be left undisturbed."
   You explain to Rebina that although you met Jorek, he didn't accompany you on your way here. Rebina gives you a grim smile in return. "Oh, I know that. I never expected you to bring him to us in the first place. You see, it's not Jorek we wanted you to get. It was... that."]]
   text[8] = [[You follow her gaze, and spot the crewmen making their way back to the airlock, carrying between them a small but apparently rather heavy crate. You are absolutely certain you've never seen it before.
   "That is what Jorek was keeping for us on %s, and that is what we need," Rebina explains. "Jorek is nothing but a decoy to draw the Empire's attention away from our real operations. While you were talking to him, his subordinates secured our cargo aboard your ship. We chose not to inform you about this because, well... It's best you didn't know what was in that crate. I'm sure we understand each other."
   Rebina turns to follow her men back into the ship, but before she closes the airlock hatch she looks back at you over her shoulder, shooting you a casual glance that nevertheless seems to see right through you. "I'm glad to see my trust in you was not misplaced," she remarks. "Perhaps we'll see each other again someday, and when we do perhaps we can do some more business."
   Then she is gone. You stare at the airlock hatch, then down at the credit chip in your hand, first marveling at the wealth it represents and then astonished to realize you can't remember how it got there.]]

   -- Mission details
   misn_title = "Shadowrun"
   misn_reward = "You were promised riches..."
   misn_desc = {}
   bar_desc = "You spot a dark-haired woman sitting at the bar. Her elegant features and dress make her stand out, yet her presence here seems almost natural, as if she's in the right place at the right time, waiting for the right person. You wonder why she's all by herself."
   misn_desc[1] = "Fly to planet %s in the %s system and talk to Jorek. Once Jorek has boarded your ship, proceed to system %s and board the %s."
   
   credits = 100000 -- 100K
   timelimit1 = 30
   timelimit2 = 70
   
   -- Aborted mission

   -- OSD stuff
   osd_title = {}
   osd_msg   = {}
   osd_title[1] = "Shadowrun"
   osd_msg[1] = "Fly to planet %s in the %s system and pick up Jorek."
   osd_msg[2] = "You have %s remaining."
   osd_msg[3] = "Fly to the %s system and dock with (board) %s"
   osd_msg[4] = "You have %s remaining."
end

function create ()
    misn.setNPC( "A dark-haired woman", "none" )
    misn.setDesc( bar_desc ) 
end

function accept()
    if var.peek("shadowrun") == 1 then
        tk.msg(title[1], text[2])
    else
        tk.msg(title[1], text[1])
    end
    if tk.yesno(title[2], text[3]) then 
        misn.accept()
        tk.msg(title[3], string.format(text[4], planetname, sysname, sysname2, shipname))
        tk.msg(title[3], string.format(text[5], planetname, timelimit1, timelimit2, shipname, shipname))

        -- Set deadlines
        deadline1 = time.get() + time.units(timelimit1)
        deadline2 = time.get() + time.units(timelimit2)
        
        misn.setTitle(misn_title)
        misn.setReward(misn_reward)
        misn.setDesc(string.format(misn_desc[1], planetname, sysname, sysname2, shipname))
        misn.osdCreate(osd_title[1], { string.format(osd_msg[1], planetname, sysname),
                                       string.format(osd_msg[2], time.str(deadline1 - time.get())),
                                       string.format(osd_msg[3], sysname2, shipname),
                                       string.format(osd_msg[4], time.str(deadline2 - time.get()))
                                     })
        misn.setMarker(sys, "misc")
        var.push("shadowrun", 2) -- Signal for helper missions and this mission
        var.push("shadowrun_planet", planetname) -- Information for helper missions
    else
        tk.msg(title[1], refusal)
        var.push("shadowrun", 1) -- For future appearances of this mission
        misn.finish(false)
    end

    hook.land("land")
    hook.takeoff("takeoff")
    hook.enter("enter")
end

function land()
end

function takeoff()
end

function enter()
    -- Deadline stuff
    if deadline1 - time.get() > 0 then
        misn.osdCreate(osd_title[1], { string.format(osd_msg[1], planetname, sysname),
                                       string.format(osd_msg[2], time.str(deadline1 - time.get())),
                                       string.format(osd_msg[3], sysname2, shipname),
                                       string.format(osd_msg[4], time.str(deadline2 - time.get()))
                                     })
    elseif deadline2 - time.get() > 0 then
        misn.osdCreate(osd_title[1], { string.format(osd_msg[3], sysname2, shipname),
                                       string.format(osd_msg[4], time.str(deadline2 - time.get()))
                                     })
        misn.setMarker(sys2, "misc")
    else
        abort()
    end

    -- Random(?) pirate attacks when get closer to your system, and heavier ones when you fly away from it after meeting SHITMAN
    if system.get():jumpDist(sys) < 3 and system.get():jumpDist(sys) > 0 and var.peek("shadowrun") == 2 then
        pilot.clear()
        pilot.toggleSpawn(false)
        pirates = pilot.add("Pirate Hyena Pack", "pirate", vec2.new(0,0), false)
    elseif system.get():jumpDist(sys) < 3 and system.get():jumpDist(sys) > 0 and var.peek("shadowrun") == 3 then
        pilot.clear()
        pilot.toggleSpawn(false)
        pirates = pilot.add("Pirate Hyena Pack", "pirate", vec2.new(0,0), false)
        pirate1 = pilot.add("Pirate Ancestor", "pirate", vec2.new(0,20), false)
        pirate2 = pilot.add("Pirate Ancestor", "pirate", vec2.new(-20,0), false)
        pirate3 = pilot.add("Pirate Ancestor", "pirate", vec2.new(0,-20), false)
    end
    
    -- Empire ships around planet
    if system.get() == sys then
        pilot.clear()
        pilot.toggleSpawn(false)
        planetpos = planet:pos()
        empire1 = pilot.add("Empire Pacifier", "empire_idle", planetpos + vec2.new(40,0), false)
        empire2 = pilot.add("Empire Pacifier", "empire_idle", planetpos + vec2.new(25,25), false)
        empire3 = pilot.add("Empire Pacifier", "empire_idle", planetpos + vec2.new(0,40), false)
        empire4 = pilot.add("Empire Pacifier", "empire_idle", planetpos + vec2.new(-25,25), false)
        empire5 = pilot.add("Empire Pacifier", "empire_idle", planetpos + vec2.new(-40,0), false)
        empire6 = pilot.add("Empire Pacifier", "empire_idle", planetpos + vec2.new(-25,-25), false)
        empire7 = pilot.add("Empire Pacifier", "empire_idle", planetpos + vec2.new(0,-40), false)
        empire8 = pilot.add("Empire Pacifier", "empire_idle", planetpos + vec2.new(25,-25), false)
    end

    -- Handle the Kestrel that's the last stop on this mission
    if var.peek("shadowrun") >= 2 and system.get() == sys2 then
        mypos = vec2.new(-1500, 600)
        targship = pilot.add("Pirate Kestrel", "def", mypos, false)
        targship[1]:rename(shipname)

        targship[1]:setFaction(faction.get("Independent")) -- Make sure the ship has no enemies here

        targship[1]:disable()
        hook.pilot(targship[1], "board", "board")
    end
end

function board()
    if var.peek("shadowrun") == 2 then
        -- player reports in without SHITMAN
        tk.msg(title[4], string.format(text[6], shipname, shipname, shipname, shipname))
        var.push("shadowrun_failed", true)
    else
        -- player reports in with SHITMAN
        tk.msg(title[5], string.format(text[7], shipname, shipname, player.name()))
        tk.msg(title[5], string.format(text[8], planetname))
        player.pay(credits)
    end

    player.unboard()
    targship[1]:setHealth(100, 100)
    targship[1]:changeAI("flee")

    var.pop("shadowrun")
    var.pop("shadowrun_planet")
    misn.finish(true)
end

function abort()
    var.push("shadowrun_failed", true)
    misn.finish(true)
end
