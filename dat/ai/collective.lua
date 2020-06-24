require("dat/ai/tpl/generic.lua")
require("dat/ai/personality/patrol.lua")

-- Settings
mem.control_rate = 0.5 -- Lower control rate
mem.aggressive = true
mem.land_planet = false

function create ()
   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system
   mem.comm_no = _("No response.")
   create_post()
end
