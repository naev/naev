include("ai/tpl/generic.lua")

--[[

   Generic Mission baddie AI

]]--

-- Settings
mem.aggressive     = true
mem.safe_distance  = 500
mem.armour_run     = 80
mem.armour_return  = 100
mem.atk_board      = true
mem.atk_kill       = true
mem.atk_board      = false
mem.bribe_no       = "You can't bribe me!"
mem.refuel_no      = "I won't give you fuel!"


function create ()

   -- Choose attack format
   attack_choose()
end