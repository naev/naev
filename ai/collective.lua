include("ai/tpl/generic.lua")

-- Settings
mem.control_rate = 0.5 -- Lower control rate
mem.aggressive = true
mem.land_planet = false

function create ()
   mem.comm_no = "No response."
   create_post()
end
