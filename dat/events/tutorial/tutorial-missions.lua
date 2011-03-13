-- This is tutorial: missions and events.

include("dat/events/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title1 = "Tutorial: Missions and events"
    message1 = [[Welcome to tutorial: Missions and events.
    
This tutorial explains missions, which are the main driving force behind the game, as well as events, which give flavor to the game.

Please note that this tutorial assumes you know how to pilot your ship. If you don't, you should consider playing the earlier tutorials first.]]
    message2 = [[A mission, as you might expect, is a task given to you, the player, to complete for some kind of reward. Missions come in two varieties, computer missions and NPC missions. Computer missions are exclusively obtained from the mission computer, and these missions are typically randomly generated. They serve to provide you with opportunities for work wherever you go.
NPC missions are given to you by game characters. This most often happens in the spaceport bar, but sometimes a mission might come to you in other ways, so keep a look out.]]
    message3 = [[We're going to accept a mission from an NPC. The NPC is waiting for you on Rin, in the spaceport bar. Land on Rin, then select the bar tab on the planet screen.]]
    message4 = [[As you can see, there's someone here in the bar. He will give you a mission if you approach him. To do so, click on his portrait first, then click on the approach button.

It's worth noting here that not all NPCs you encounter in the spaceport bars will give you missions. Those that do tend to look slightly different though, and they tend to be grouped near the beginning of the NPC list.]]
    
    landomsg = "Land on Rin and visit the spaceport bar"
end

function create()
    -- Set up the player here.
    player.teleport("Navajo")
    player.pilot():setPos(planet.get("Rin"):pos() + vec2.new(0, 250))
    player.msgClear()
    
    system.get("Mohawk"):setKnown(true)
    system.get("Cherokee"):setKnown(true)
    system.get("Iroquois"):setKnown(true)

    -- All input available for now.

    tk.msg(title1, message1)
    tk.msg(title1, message2)
    tk.msg(title1, message3)

    omsg = player.omsgAdd(landomsg, 0)
    hook.land("land")
    hook.land("bar", "bar")
    hook.takeoff("takeoff")
end

-- Land hook.
function land()
    if planet.cur() == planet.get("Rin") then
        if not (player.misnActive("Tutorial Mission") or player.misnDone("Tutorial Mission")) then
            tutNPC = evt.npcAdd("NPC", "Tutorial employee", "thief1", "This person has a mission for you!")
        end
    end
end

-- Helper land hook.
function bar()
    if seen then return
    else
        tk.msg(title1, message4)
        seen = true
    end
end

-- NPC hook
function NPC()
    player.omsgRm(omsg)
    naev.missionStart("Tutorial Mission")
    evt.npcRm(tutNPC)
end

-- Takeoff hook.
function takeoff()
    if player.misnDone("Tutorial Mission") then
        cleanup()
    end
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    naev.keyEnableAll()
    naev.eventStart("Tutorial")
    evt.finish(true)
end