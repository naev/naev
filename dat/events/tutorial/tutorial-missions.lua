-- This is tutorial: missions and events.

include("dat/events/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
title1 = _("Tutorial: Missions and Events")
message1 = _([[Welcome to the missions and events tutorial.

This tutorial explains missions, which are the main driving force behind the game, as well as events, which give flavor to the game.

Please note that this tutorial assumes you know how to pilot your ship. If you don't, you should consider playing the earlier tutorials first.]])
message2 = _([[A mission, as you might expect, is a task given to you, the player, to complete for some kind of reward. Missions come in two varieties, computer missions and NPC missions. Computer missions are exclusively obtained from the mission computer, and these missions are typically randomly generated. They serve to provide you with opportunities for work wherever you go.
NPC missions are given to you by game characters. This most often happens in the spaceport bar, but sometimes a mission might come to you in other ways, so keep a look out.]])
message3 = _([[We're going to accept a mission from an NPC. The NPC is waiting for you on Rin, in the spaceport bar. Land on Rin, then select the bar tab on the planet screen.]])
message4 = _([[As you can see, there's someone here in the bar. He will give you a mission if you approach him. To do so, click on his portrait first, then click on the approach button.

It's worth noting here that not all NPCs you encounter in the spaceport bars will give you missions. Those that do tend to look slightly different though, and they tend to be grouped near the beginning of the NPC list.]])
message5 = _([[Now that you know how missions work, it's easy to explain what events are. Events work much the same as missions, but they happen by themselves. You don't get to search them out, you don't get to accept or decline them, nor can you abort them. If an event happens to you, you're just going to have to ride it out, for better or for worse. In fact, one is happening to you right now, since this tutorial itself is an event as well!]])
message6 = _([[You now know how to play missions. Missions are where most of the action in the game happens, and your career will depend greatly on which missions you choose to accept. As a final tip, some missions only appear in certain areas of space, so be sure to travel around a lot and check the spaceport bars wherever you go.

Congratulations! This concludes the missions and events tutorial.]])

landomsg = _("Land on Rin and visit the spaceport bar")

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
    hook.enter("enter")
end

-- Land hook.
function land()
    if planet.cur() == planet.get("Rin") then
        if not (player.misnActive("Tutorial Mission") or player.misnDone("Tutorial Mission")) then
            tutNPC = evt.npcAdd("NPC", _("Tutorial employee"), "neutral/thief1", _("This person has a mission for you!"))
        end
    end
end

-- Helper land hook.
function bar()
    if seen or planet.cur() ~= planet.get("Rin") then return
    else
        tk.msg(title1, message4)
        seen = true
    end
end

-- NPC hook
function NPC()
    player.omsgRm(omsg)
    naev.missionStart("Tutorial Mission")
    if player.misnActive("Tutorial Mission") then -- He COULD have declined.
        evt.npcRm(tutNPC)
    end
end

-- Enter hook, for fuel cheat.
function enter()
    player.pilot():setFuel(true)
end

-- Takeoff hook.
function takeoff()
    if player.misnDone("Tutorial Mission") then
        tk.msg(title1, message5)
        tk.msg(title1, message6)
        hook.safe("cleanup")
    end
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    if not (omsg == nil) then player.omsgRm(omsg) end
    naev.eventStart("Tutorial")
    evt.finish(true)
end
