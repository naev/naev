require("ai/tpl/generic")
require("ai/personality/patrol")

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
mem.bribe_no       = _("You can't bribe me!")
mem.refuel_no      = _("I won't give you fuel!")


function create ()

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system

   -- Choose attack format
   attack_choose()
end
