require("ai/tpl/escort")
require("ai/personality/patrol")

-- Settings
mem.aggressive = true
mem.doscans = false

function create ()
   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system
end

