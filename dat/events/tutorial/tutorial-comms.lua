-- This is tutorial: communications.

include("dat/events/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title1 = "Tutorial: Communications"
    message1 = [[Welcome to the communications tutorial.
    
This tutorial will show you how you can contact other ships, as well as planets and stations.]]
    message2 = [[Your ship comes with a built-in communications array. To use it, target whatever you want to contact (be it a ship or a planet) and press %s. But be careful, if you have both a planet and a ship targeted, you will contact the ship rather than the planet. If you want to speak to a planet, you should first clear your target with %s.

To proceed to the next step, target the nearby ship and contact it. You may contact the planet first if you wish, to see what options are available when you do so.]]
    message3 = [[Well done. Normally, when hailing a ship, you may ask it to sell you fuel or bribe it if it's attacking you, but that's about all you can do. However, sometimes a mission requires you to contact a ship in order to proceed. Since this is a drone, there's not much you can say to it.
    You may also try to bribe planets by contacting them. This is a good way to get to planets that would otherwise deny you access. Note that you can't bribe planets that will let you land in the first place.]]
    message4 = [[You now know how to use the communications system, so I suppose that's it for this tutorial...

But wait, what's this? A ship is taking off from the planet below, and it seems it's interested in talking to you!]]
    message5 = [[As you just witnessed, ships that are trying to contact you will have an animated icon next to them. You will also be notified in the message log, and a sound will alert you that someone's hailing you.

Let's respond to the ship. You could do this in the same way you just learned, by targeting the ship and pressing %s, but that can be inconvenient when there are many ships around. An easy way to respond to the incoming call is by using the auto-respond shortcut, which is %s. Try answering the ship now.]]
    message6 = [[Excellent. Ships won't often hail you, but when they do it's highly likely they have something important to say. Keep a look out for ships trying to contact you!]]
    message7 = [[You now know how to talk to ships and planets, and you know what to do when ships want to talk to you. As a final tip, remember that auto-responding to a ship will automatically target it, so be sure not to accidentally attack it!

Congratulations! This concludes the communications tutorial.]]

    hailomsg = "Target the drone and use %s to contact it"
    autohailomsg = "Use %s to respond to the hail "
end


function create()
    -- Set up the player here.
    player.teleport("Mohawk")
    pp = player.pilot()
    pp:setPos(planet.get("Paul 2"):pos() + vec2.new(0, 250))
    player.msgClear()
    
    pilot.clear()
    pilot.toggleSpawn(false)
    commdrone = pilot.add("Civilian Llama", "dummy", pp:pos() + vec2.new(0, 250))[1]
    commdrone:setVisplayer()
    
    player.pilot():setNoLand()
    player.pilot():setNoJump()
    
    tk.msg(title1, message1)
    tk.msg(title1, message2:format(tutGetKey("hail"), tutGetKey("target_clear")))
    
    dronehook = hook.pilot(commdrone, "hail", "haildrone")
    omsg = player.omsgAdd(hailomsg:format(tutGetKey("hail")), 0)
end

-- Hail hook.
function haildrone()
    hook.rm(dronehook)
    player.commClose()
    player.omsgRm(omsg)

    tk.msg(title1, message3)
    tk.msg(title1, message4)
    commship = pilot.add("Civilian Gawain", "dummy", planet.get("Paul 2"))[1]
    commship:setVisplayer()
    commship:hailPlayer()
    hook.timer(4000, "shiptakeoff")
end

-- Timer hook to allow the ship to take off fully.
function shiptakeoff()
    commship:hailPlayer()
    tk.msg(title1, message5:format(tutGetKey("hail"), tutGetKey("autohail")))
    omsg = player.omsgAdd(autohailomsg:format(tutGetKey("autohail")), 0)
    shiphook = hook.pilot(commship, "hail", "hailship")
end

-- Hail hook.
function hailship()
    hook.rm(shiphook)
    player.commClose()
    tk.msg(title1, message6)
    tk.msg(title1, message7)
    hook.safe( "cleanup" )
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    if not (omsg == nil) then player.omsgRm(omsg) end
    naev.keyEnableAll()
    naev.eventStart("Tutorial")
    evt.finish(true)
end