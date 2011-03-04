-- This is the first tutorial: basic operation.

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title1 = "Tutorial: Basic operation"
    message1 = [[Welcome to tutorial: Basic operation.

This tutorial will teach you what Naev is about, and show you the elementary controls of your ship.]]
    message2 = [[We will start by flying around. Use %s and %s to turn, and %s to accelerate. Try flying around the planet.]]
    message3 = [[Flying is easy, but stopping is another thing. To stop, you will need to thrust in the direction you're heading in. To make this task easier, you can use %s to reverse your direction. Once you have turned around completely, thrust to decrease your speed. Try this now.]]
    message4 = [[Well done. Maneuvering and stopping will be important for playing the game.
    
During the game, however, you will often need to travel great distances within a star system. To make this easier, you can use the overlay system map. It is accessed with %s. Open the overlay map now.]]
    
    flyomsg = "Fly around (%ds remaining)"
    stopomsg = "Press and hold %s until you stop turning, then thrust until you come to a (near) stop"
    mapomsg = "Press %s to open the overlay map"
end

function create()
    misn.accept()

    -- Set up the player here.
    player.teleport("Mohawk")

    player.pilot():setPos(planet.get("Paul 2"):pos() + vec2.new(0, 250))
    -- TODO: Disable all player input save for basic maneuvering (not turnaround).

    tk.msg(title1, message1)
    tk.msg(title1, message2:format(tutGetKey("left"), tutGetKey("right"), tutGetKey("accel")))
    
    flytime = 10 -- seconds of fly time
    
    omsg = player.omsgAdd(flyomsg:format(flytime), 0)
    hook.timer(1000, "flyUpdate")
end

function flyUpdate()
    flytime = flytime - 1
    
    if flytime == 0 then
        player.omsgRm(omsg)
        tk.msg(title1, message3:format(tutGetKey("reverse")))
        -- TODO: Enable turnaround
        omsg = player.omsgAdd(stopomsg:format(tutGetKey("reverse")), 0)
        braketime = 0 -- ticks for brake check.
        hook.timer(500, "checkBrake")
    else
        player.omsgChange(omsg, flyomsg:format(flytime), 0)
        hook.timer(1000, "flyUpdate")
    end
end

function checkBrake()
    if player.pilot():vel():mod() < 50 then
        braketime = braketime + 1
    else
        braketime = 0
    end
    
    if braketime > 4 then
        -- Have been stationary (or close enough) for long enough
        player.omsgRm(omsg)
        tk.msg(title1, message4:format(tutGetKey("overlay")))
        omsg = player.omsgAdd(mapomsg:format(tutGetKey("overlay")), 0)
        -- TODO: Enable overlay map, disable regular navigation
        -- TODO: Input hook!
    else
        hook.timer(500, "checkBrake")
    end
end

-- Capsule function for naev.getKey() that adds a color code to the return string.
function tutGetKey(command)
    return "\027b" .. naev.getKey(command) .. "\0270"
end

-- Abort hook.
function abort()
    cleanup()
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    -- Function to return to the tutorial menu here
end