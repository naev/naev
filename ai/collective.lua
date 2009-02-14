include("ai/tpl/generic.lua")

-- Settings
control_rate = 0.5 -- Lower control rate
aggressive = true
land_planet = false

function create ()
   mem.comm_no = "No response."
   attack_choose()
end
