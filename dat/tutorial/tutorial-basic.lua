-- This is the first tutorial: basic operation.

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title1 = "Tutorial: Basic operation"
    message1 = [[Welcome to tutorial: Basic operation.

This tutorial will teach you what Naev is about, and show you the elementary controls of your ship.]]
    message2 = [[We will start by flying around. Use %s and %s to turn, and %s to accelerate. Try flying around the planet.]]
    
    noland = "You may not land yet."
    flyomsg = "Fly around (%ds remaining)"
end

function create()
    misn.accept()

    -- Set up the player here.
    player.teleport("Mohawk")
    diff.apply("Tutorial jumps")

    player.pilot():setPos(planet.get("Paul 2"):pos() + vec2.new(0, 250))
    player.allowLand(false, noland)

    tk.msg(title1, message1:format(naev.getKey("left"), naev.getKey("right"), naev.getKey("accel")))
    
    flytime = 10 -- seconds of fly time
    
    omsg = player.omsgAdd(flyomsg:format(flytime), 9999)
    hook.timer(1000, "flyUpdate")
end

function flyUpdate()
    flytime = flytime - 1
    
    if flytime < 1 then
        player.omsgRm(omsg)
    else
        player.omsgChange(omsg, flyomsg:format(flytime), 9999)
        hook.timer(1000, "flyUpdate")
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