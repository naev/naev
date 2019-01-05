-- This is the first tutorial: basic operation.

include("proximity.lua")
include("dat/events/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
title1 = _("Tutorial: Basic Operation")
message1 = _([[Welcome to the basic operation tutorial.

This tutorial will teach you what Naev is about, and show you the elementary controls of your ship.]])
message2 = _([[We will start by flying around. Use %s and %s to turn, and %s to accelerate. Try flying around the planet.]])
message3 = _([[Flying is easy, but stopping is another thing. To stop, you will need to thrust in the opposite of the direction you're heading in. To make this task easier, you can use %s to reverse your direction. Once you have turned around completely, thrust to decrease your speed. Try this now.]])
message4 = _([[Well done. For convenience, it's also possible to automatically turn around and stop by pressing %s.

Note that your ship can also fly towards the mouse, which can be toggled with %s or clicking your middle mouse button.

During the game, however, you will often need to travel great distances within a star system. To make this easier, you can use the overlay system map. It is accessed with %s. Open the overlay map now.]])
message5 = _([[This is the system overlay map. It displays an overview of the star system you're currently in, displaying planets, jump points and any ships your scanners are currently detecting.
You can use the overlay map to navigate around the system. Right click on a location to make your ship automatically fly there. Time will speed up during the journey, so you'll be there shortly.

There is a marker on the map. Order your ship to fly to it. You can close the overlay map once you're underway if you wish.]])
message6 = _([[Excellent. Autopilot navigation in time compression is the most convenient way to get around in a system. Note that the autopilot will NOT stop you, you need to do that yourself.]])
message7 = _([[As you can see, there is another ship here. We're going to board it. For this, you must do three things.
- First, target the ship. You can do this with %s, or by clicking on the ship.
- Then, come to a (near) stop right on top of the ship. You learned how to do this earlier.
- Finally, use %s to board the ship.]])
message8 = _([[You have successfully boarded the ship. Boarding is useful in a number of situations, for example when you want to steal cargo or credits from a ship you've disabled in combat, or if a ship is asking for help.]])
message9 = _([[The final step in this tutorial is landing. Landing works the same way as boarding, but with planets and stations. Target a planet with %s or the mouse, slow to a stop over the planet or station, then press %s or double-click on the planet to land.

Land on Paul 2 now. Remember, you can use the overlay map to get there quicker!]])
message10 = _([[Good job, you have landed on Paul 2. Your game will automatically be saved whenever you land. As a final tip, you can press %s even if you haven't targeted a planet or station - you will automatically target the nearest landable one.

Congratulations! This concludes the basic operation tutorial.]])

flyomsg = _("Fly around (%ds remaining)")
stopomsg = _("Press and hold %s until you stop turning, then thrust until you come to a (near) stop")
mapomsg = _("Press %s to open the overlay map")
boardomsg = _("Target the ship with %s, then approach it and press %s to board")
landomsg = _("Target Paul 2 with %s, then request landing permission with %s. Once granted, press %s again to land")

function create()
    -- Set up the player here.
    player.teleport("Mohawk")
    player.msgClear()

    pilot.clear()
    pilot.toggleSpawn(false) -- To prevent NPCs from getting targeted for now.
    player.pilot():setPos(planet.get("Paul 2"):pos() + vec2.new(0, 250))
    
    player.pilot():setNoLand()
    player.pilot():setNoJump()
    
    tk.msg(title1, message1)
    tk.msg(title1, message2:format(tutGetKey("left"), tutGetKey("right"), tutGetKey("accel")))
    
    flytime = 10 -- seconds of fly time
    
    omsg = player.omsgAdd(flyomsg:format(flytime), 0)
    hook.timer(1000, "flyUpdate")
end

-- Allow the player to fly around as he likes for 10s.
function flyUpdate()
    flytime = flytime - 1
    
    if flytime == 0 then
        player.omsgRm(omsg)
        tk.msg(title1, message3:format(tutGetKey("reverse")))

        omsg = player.omsgAdd(stopomsg:format(tutGetKey("reverse")), 0)
        braketime = 0 -- ticks for brake check.
        hook.timer(500, "checkBrake")
    else
        player.omsgChange(omsg, flyomsg:format(flytime), 0)
        hook.timer(1000, "flyUpdate")
    end
end

-- Check if the player has managed to stop.
function checkBrake()
    if player.pilot():vel():mod() < 50 then
        braketime = braketime + 1
    else
        braketime = 0
    end
    
    if braketime > 4 then
        -- Have been stationary (or close enough) for long enough
        player.omsgRm(omsg)
        tk.msg(title1, message4:format(tutGetKey("autobrake"),
            tutGetKey("mousefly"), tutGetKey("overlay")))
        omsg = player.omsgAdd(mapomsg:format(tutGetKey("overlay")), 0)
        player.pilot():setVel(vec2.new()) -- Stop the player completely
        waitmap = true
        hook.input("input")
    else
        hook.timer(500, "checkBrake")
    end
end

-- Input hook.
function input(inputname, inputpress)
    if waitmap and inputname == "overlay" then
        player.omsgRm(omsg)
        tk.msg(title1, message5)
        targetpos = vec2.new(-3500, 3500) -- May need an alternative?
        marker = system.mrkAdd("Fly here", targetpos)
        waitmap = false

        boardee = pilot.add("Civilian Gawain", nil, targetpos)[1]
        boardee:disable()
        hook.pilot(boardee, "board", "board")

        proximity({location = targetpos, radius = 350, funcname = "proxytrigger"})
    end
end

-- Function that runs when the player approaches the indicated coordinates.
function proxytrigger()
    system.mrkClear()
    tk.msg(title1, message6)
    tk.msg(title1, message7:format(tutGetKey("target_next"), tutGetKey("board")))
    omsg = player.omsgAdd(boardomsg:format(tutGetKey("target_next"), tutGetKey("board")), 0)
end

-- Board hook for the board practice ship.
function board()
    player.unboard()
    tk.msg(title1, message8)
    tk.msg(title1, message9:format(tutGetKey("target_planet"), tutGetKey("land"), tutGetKey("land")))
    player.omsgChange(omsg, landomsg:format(tutGetKey("target_planet"), tutGetKey("land"), tutGetKey("land")), 0)
    hook.land("land")
    player.pilot():setNoLand(false)
end

-- Land hook.
function land()
    tk.msg(title1, message10:format(tutGetKey("land")))
    player.takeoff()
    hook.safe( "cleanup" )
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    if not (omsg == nil) then player.omsgRm(omsg) end
    naev.keyEnableAll()
    naev.eventStart("Tutorial")
    evt.finish(true)
end
