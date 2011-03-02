-- This is the first tutorial: basic operation.

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title1 = "Tutorial: Basic operation"
    message1 = [[Welcome to tutorial: Basic operation.

This tutorial will teach you what Naev is about, and show you the elementary controls of your ship.]]
    message2 = [[We will start by flying around. Use %s and %s to turn, and %s to accelerate. Try flying around the planet.]]
    message3 = [[Flying is easy, but stopping is another thing. To stop, you will need to thrust in the direction you're heading in. To make this task easier, you can use the %s button to reverse your direction. Once you have turned around completely, thrust to decrease your speed. Try this now.]]
    
    noland = "You may not land yet."
    flyomsg = "Fly around (%ds remaining)"
    stopomsg = "Hold down %s until you stop turning, then thrust until you come to a (near) stop"
end

function create()
    misn.accept()

    -- Set up the player here.
    player.teleport("Mohawk")
    diff.apply("Tutorial jumps")

    player.pilot():setPos(planet.get("Paul 2"):pos() + vec2.new(0, 250))
    player.allowLand(false, noland)
    -- TODO: Disable all player input save for basic maneuvering (not turnaround).

    tk.msg(title1, message1)
    tk.msg(title1, message2:format(naev.getKey("left"), naev.getKey("right"), naev.getKey("accel")))
    
    flytime = 10 -- seconds of fly time
    
    omsg = player.omsgAdd(flyomsg:format(flytime), 0)
    hook.timer(1000, "flyUpdate")
end

function flyUpdate()
    flytime = flytime - 1
    
    if flytime == 0 then
        player.omsgRm(omsg)
        tk.msg(title1, message3:format(naev.getKey("reverse")))
        -- TODO: Enable turnaround
        omsg = player.omsgAdd(flyomsg:format(naev.getKey("reverse")), 0)
        braketime = 0 -- ticks for brake check.
        hook.timer(500, "checkBrake")
    else
        player.omsgChange(omsg, flyomsg:format(flytime), 0)
        hook.timer(1000, "flyUpdate")
    end
end

function checkBrake()
    if player.pilot():vel():mod() < 10 then
        braketime = braketime + 1
    else
        braketime = 0
        hook.timer(500, "checkBrake")
    end
    
    if braketime > 2 then
        -- Have been stationary (or close enough) for long enough
        player.omsgRm(omsg)
    end
end

-- Abort hook.
function abort()
    cleanup()
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    -- Function to return to the tutorial menu here
end