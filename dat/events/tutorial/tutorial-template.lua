-- Template for tutorial modules.
-- Each module should start by setting up the tutorial environment and enforcing rules.
-- Each module should clean up, set mission variable "var_next" to next tutorial in sequence, and return to the main tutorial menu when ending or aborting.

include("dat/events/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    
end

function create()
    -- Set up the player here.
    player.teleport("Mohawk")
    player.pilot():setPos(vec2.new(0, 0))
    player.msgClear()
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    var.push("tut_next", "Next Tutorial")
    naev.keyEnableAll()
    naev.eventStart("Tutorial")
    evt.finish(true)
end
