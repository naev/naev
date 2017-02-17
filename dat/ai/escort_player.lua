include("dat/ai/tpl/escort.lua")
include("dat/ai/personality/patrol.lua")

-- Settings
mem.aggressive = true


function create ()
   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system
end

