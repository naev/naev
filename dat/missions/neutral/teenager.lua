--[[
--
-- MISSION: The macho teenager
-- DESCRIPTION: A man tells you that his son has taken one of his yachts without permission and
-- is joyriding it with his girlfriend to impress her. Disable the yacht and board it, then take
-- the couple back to the planet (destroying the yacht incurs a penalty)
--
--]]

-- Localization, choosing a language if naev is translated for non-english-speaking locales.
lang = naev.lang()
if lang == "es" then
else -- Default to English

text = {}
title = {}

    title[1] = "Youngsters these days"
    text[1] = [[    "Excuse me," the man says as you approach him. "I'm looking for a capable pilot to resolve a small matter for me. Perhaps you can help me? You see, it's my son. He's taken my yacht to space without my permission, taking along his girlfriend. That boy is such a handful. I'm sure he's trying to show off his piloting skills to impress her. I need you to get out there, disable the yacht and take them both back here. Can you do this for me? I'll make it worth your while."]]

    title[2] = "It's a lousy job, but..."
    text[2] = [[    "Thank you! The yacht doesn't have a working hyperdrive, so they won't have left the system. It's a Gawain named Credence. Just disable it and board it, then transport my idiot son and his girlfriend back here. Don't worry about the yacht, I'll have it recovered later. Oh, and one more thing, though it should go without saying: whatever you do, don't destroy the yacht! I don't want to lose my son over this. Well then, I hope to see you again soon."]]

    title[3] = "Whoops!"
    text[3] = [[You have destroyed the Gawain! The family presses charges, and you are sentenced to a %d fine in absence of attendance.]]

    title[4] = "End of the line, boyo"
    text[4] = [[    You board the Gawain and find an enraged teenage boy and a disillusioned teenage girl. The boy is furious that you attacked and disabled his ship, but when you mention that his father is quite upset and wants him to come home right now, he quickly pipes down. You march the young couple onto your ship and seal the airlock behind you.]]

    title[5] = "You're grounded, young man"
    text[5] = [[    The boy's father awaits you at the spaceport. He gives his son and the young lady a stern look and curtly commands them to wait for him in the spaceport hall. The couple droops off, and the father turns to face you.
    "You've done me a service, sir," he says. "As promised, I have a reward for a job well done. You'll find it in your bank account. I'm going to give my son a reprimand he'll not soon forget, so hopefully he won't repeat this little stunt anytime soon. Well then, I must be going. Thank you again, and good luck on your travels."]]

    NPCname = "A middle-aged man"
    NPCdesc = "You see a middle-aged man, who appears to be one of the locals, looking around the bar, apparently in search of a suitable pilot."

    misndesc = "A disgruntled parent has asked you to fetch his son and his son's girlfriend, who have taken a yacht and are joyriding it in the %s system."
    misnreward = "You will be compensated for your efforts."

    OSDtitle = "The macho teenager"
    OSD = {}
    OSD[1] = "Disable Gawain Credence"
    OSD[2] = "Bring the teenagers back to planet %s"

end


function create ()
    cursys = system.cur()
    curplanet = planet.cur()
    OSD[2] = OSD[2]:format(planet.cur():name())
    misn.setNPC(NPCname, "none") -- TODO: portrait
    misn.setDesc(NPCdesc)
end


function accept ()
    if tk.yesno(title[1], text[1]) then
        misn.accept()
        misn.setDesc(misndesc:format(cursys:name()))
        misn.setReward(misnreward)
        misn.osdCreate(OSDtitle, OSD)
        tk.msg(title[2], text[2])
        hook.enter("enter")
        targetlive = true
    else
        misn.finish()
    end
end

function enter()
    if system.cur() == cursys and targetlive then
        dist = rnd.rnd() * system.cur():radius()
        angle = rnd.rnd() * 2 * math.pi
        location = vec2.new(dist * math.cos(angle), dist * math.sin(angle)) -- Randomly spawn the Gawain in the system
        target = pilot.add("Civilian Gawain", nil, location)[1]
        target:control()
        target:rename("Credence")
        target:setHilight(true)
        target:setVisible(true)
        hook.pilot(target, "idle", "targetIdle")
        hook.pilot(target, "death", "targetDeath")
        hook.pilot(target, "board", "targetBoard")
        targetIdle()
    end
end

function targetIdle()
    location = target:pos()
    dist = 500
    angle = rnd.rnd() * 2 * math.pi
    newlocation = vec2.new(dist * math.cos(angle), dist * math.sin(angle)) -- New location is 500px away in a random direction
    target:taskClear()
    target:goto(location + newlocation, false, false)
end

function targetDeath()
    fine = math.max(-20000, -player.credits()) -- Fine 20K, or take the player for all he has
    tk.msg(title[3], text[3]:format(-fine))
    player.pay(fine) -- I love this statement.
    misn.finish(true)
end

function targetBoard()
    player.unboard()
    tk.msg(title[4], text[4])
    target:setHilight(false)
    -- TODO: Add teenagers mission cargo here.
    misn.osdActive(2)
    hook.land("land")
end

function land()
    if planet.cur() == curplanet then
        tk.msg(title[5], text[5])
        player.pay(30000) -- 30K
        misn.finish(true)
    end
end

function abort ()
   misn.finish(false)
end
