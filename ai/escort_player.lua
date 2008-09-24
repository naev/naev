include("ai/tpl/escort.lua")

-- Settings
armour_run = 40
armour_return = 70
aggressive = true
command = false


function create ()
   mem.escort = ai.getPlayer()
end

