include("ai/tpl/escort.lua")
include("ai/personality/patrol.lua")

-- Settings
mem.armour_run = 40
mem.armour_return = 70
mem.aggressive = true
mem.command = false


function create ()
   mem.escort = ai.getPlayer()
end

