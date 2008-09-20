include("ai/tpl/escort.lua")

-- Settings
armour_run = 40
armour_return = 70
aggressive = true


function create ()
   mem.escort = ai.getPlayer()
end

