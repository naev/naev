-- This is the first tutorial: basic operation.

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title1 = "Tutorial: Basic operation"
    message1 = [[Welcome to tutorial: Basic operation.

This tutorial will teach you what Naev is about, and show you the elementary controls of your ship.]]
    message2 = [[We will start by flying around.]]
    
    noland = "You may not land yet."
end

function create()
    -- Set up the player here.
    player.teleport("Mohawk")
    diff.apply("Tutorial jumps")

    player.pilot():setPos(planet.get("Paul 2"):pos() + vec2.new(0, 250))
    player.allowLand(false, noland)
    player.control()

    tk.msg(title1, message1)
end

-- Abort hook.
function abort()
    cleanup()
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    -- Function to return to the tutorial menu here
end