-- This is the mission used in tutorial: missions and events.

include("proximity.lua")
include("dat/events/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
title1 = _("Mission")
message1 = _([[    "Hello there guv," the man says to you. "You're the guy doing the tutorial, right? Good, then let me give you a short explanation on how this works.
    "See, when you talk to people in the bar like this, and they have a mission to offer you, they'll usually tell you what the mission is about, in general terms. Listen to what they have to say, because you can usually tell what kind of mission it's going to be. Usually, the mission giver will ask you if you want to do the mission at the end. If you think you can't handle it, you can always decline! Don't worry, you'll be able to find the mission giver again later, so no pressure, right?
    "Okay, so let's try it for reals. The mission I'm sending you on is information gathering. Easy peasy. You just need to fly to the Cherokee system, there's an old, abandoned station there. You need to fly to it and come to a stop close to it so your sensors can scan it. Once you do that, fly to Paul 2 in the Mohawk system, I'll meet you there. Can you do that?"]])
declinemessage = _([[    "Okay, but you can't complete the tutorial if you don't. Like I said, you can just talk to me again and I'll tell you the same thing I just did, like a broken record."]])
message2 = _([[    "Great! Now, since you've accepted this mission, you will see your mission objectives printed on the screen when in space. Your current objective will be highlighted in green. Your first objective is to fly to the abandoned station. Like I said, it's in the Cherokee system. I have put a marker next to that system on your galaxy map, so you know where to go. Missions tend to do this, so keep a look out. Once in Cherokee, you can find the station with your overlay map.
    "Oh, one more thing. When you're doing a mission and it's going badly, you can abort the mission at any time. Just open your info menu, select the missions tab, then click on the mission and hit the abort button. Most missions will just re-appear if you do this, so there's no need to worry about it too much. That goes for this mission too. If you abort it, just come find me again here.]])
message3 = _([[Your scanning equipment does a quick sweep of the abandoned station. There's nothing of importance left there, it seems.]])
message4 = _([[    The man who gave you the mission meets you at the space dock. "Hello again guv," he greets you. "As you can see, you don't necessarily need to talk to mission givers again if you want to complete a mission. Usually it's enough to land on the planet they want you to go to, or fulfill some other condition.
    "Normally, this is where you get your reward, but seeing how this is the tutorial, you wouldn't have much use for it. But in your future travels, you will be able to make some serious money this way, or possibly even get ships or equipment as a reward.
    "Well, that's all there is to it. Just take off again to complete the tutorial, okay? Good job, and good luck out there!"
    With that, the man walks back into the spaceport complex, leaving you with the satisfaction that you've successfully completed a mission.]])

misn_desc = _("You've been asked to fly to the abandoned station in the Cherokee system and come to a stop close to it.")
osd_title = _("Tutorial mission")
osd_msg = { _("Fly to the Cherokee system"),
            _("Come to a (near) stop close to the station"),
            _("Wait until the scan is complete"),
            _("Land on Paul 2 in the Mohawk system")
          }

function create()
    if tk.yesno(title1, message1) then
        accept()
    else
        tk.msg(title1, declinemessage)
        abort()
    end
end

function accept()
    tk.msg(title1, message2)
    misn.accept()

    misn.setDesc(misn_desc)
    misn.osdCreate(osd_title, osd_msg)
    marker = misn.markerAdd(system.get("Cherokee"), "high")

    pp = player.pilot()
    abstat = planet.get("Abandoned Station")
    hook.land("land")
    hook.enter("enter")
end

-- Land hook.
function land()
    if planet.cur() == planet.get("Paul 2") and scanned then
        tk.msg(title1, message4)
        misn.finish(true)
    end
end

-- Enter hook.
function enter()
    if system.cur() == system.get("Cherokee") then
        misn.osdActive(2)
        inrange = 0
        proximity({location = abstat:pos(), radius = 400, funcname = "proxytrigger"})
    elseif not scanned then
        misn.osdActive(1)
    end
end

-- Proximity hook.
function proxytrigger()
    inrange = inrange + 1
    if inrange == 7 then
        tk.msg(title1, message3)
        misn.markerMove(marker, system.get("Mohawk"))
        scanned = true
        misn.osdActive(4)
        return
    end
    if player.pilot():pos():dist(abstat:pos()) <= 400 and pp:vel():mod() < 50 then
        misn.osdActive(3)
        hook.timer(500, "proxytrigger")
    else
        inrange = 0
        misn.osdActive(2)
        hook.timer(500, "proximity", {location = abstat:pos(), radius = 400, funcname = "proxytrigger"})
    end
end

function abort()
    misn.finish(false)
end
