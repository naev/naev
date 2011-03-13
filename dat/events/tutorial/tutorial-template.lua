-- Template for tutorial modules.
-- Each module should start by setting up the tutorial environment and enforcing rules.
-- Each module should clean up and return to the main tutorial menu when ending or aborting.

include("dat/events/tutorial/tutorial-common.lua")

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    
end

function create()
    -- Set up the player here.
    player.teleport("Mohawk")
end

-- Cleanup function. Should be the exit point for the module in all cases.
function cleanup()
    naev.keyEnableAll()
    naev.eventStart("Tutorial")
    evt.finish(true)
end